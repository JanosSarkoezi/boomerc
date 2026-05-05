#include "screenshot.h"
#include <stdio.h>
#include <stdlib.h>

Screenshot new_screenshot(Display *display, Window window) {
    XWindowAttributes attributes;
    XGetWindowAttributes(display, window, &attributes);

    Screenshot result;
    result.image = XGetImage(
        display, window,
        0, 0,
        attributes.width,
        attributes.height,
        AllPlanes,
        ZPixmap);
    return result;
}

void screenshot_destroy(Screenshot *screenshot, Display *display) {
    (void) display;
    if (screenshot->image) {
        XDestroyImage(screenshot->image);
        screenshot->image = NULL;
    }
}

void screenshot_refresh(Screenshot *screenshot, Display *display, Window window) {
    XWindowAttributes attributes;
    XGetWindowAttributes(display, window, &attributes);

    XImage *refreshed_image = XGetSubImage(
        display, window,
        0, 0,
        screenshot->image->width,
        screenshot->image->height,
        AllPlanes,
        ZPixmap,
        screenshot->image,
        0, 0);

    if (refreshed_image == NULL ||
        refreshed_image->width != attributes.width ||
        refreshed_image->height != attributes.height) {
        
        XImage *new_image = XGetImage(
            display, window,
            0, 0,
            attributes.width,
            attributes.height,
            AllPlanes,
            ZPixmap);

        if (new_image != NULL) {
            XDestroyImage(screenshot->image);
            screenshot->image = new_image;
        }
    } else {
        screenshot->image = refreshed_image;
    }
}
