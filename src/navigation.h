#ifndef NAVIGATION_H_
#define NAVIGATION_H_

#include "la.h"
#include "config.h"
#include <X11/Xlib.h>
#include <stdbool.h>

typedef struct {
    Vec2f curr;
    Vec2f prev;
    bool drag;
} Mouse;

typedef struct {
    Vec2f position;
    Vec2f velocity;
    float scale;
    float delta_scale;
    Vec2f scale_pivot;
} Camera;

Vec2f camera_world(Camera camera, Vec2f v);
void camera_update(Camera *camera, Config config, float dt, Mouse mouse, XImage *image, Vec2f window_size);

#endif // NAVIGATION_H_
