#include "navigation.h"
#include <math.h>

#define VELOCITY_THRESHOLD 15.0f

Vec2f camera_world(Camera camera, Vec2f v) {
    return vec2_div_scalar(v, camera.scale);
}

void camera_update(Camera *camera, Config config, float dt, Mouse mouse, XImage *image, Vec2f window_size) {
    (void) image; // Currently unused in Nim as well, but kept in signature

    if (fabsf(camera->delta_scale) > 0.5f) {
        Vec2f p0 = vec2_div_scalar(vec2_sub(camera->scale_pivot, vec2_mul_scalar(window_size, 0.5f)), camera->scale);
        camera->scale = fmaxf(camera->scale + camera->delta_scale * dt, config.min_scale);
        Vec2f p1 = vec2_div_scalar(vec2_sub(camera->scale_pivot, vec2_mul_scalar(window_size, 0.5f)), camera->scale);
        camera->position = vec2_add(camera->position, vec2_sub(p0, p1));

        camera->delta_scale -= camera->delta_scale * dt * config.scale_friction;
    }

    if (!mouse.drag && (vec2_length(camera->velocity) > VELOCITY_THRESHOLD)) {
        camera->position = vec2_add(camera->position, vec2_mul_scalar(camera->velocity, dt));
        camera->velocity = vec2_sub(camera->velocity, vec2_mul_scalar(camera->velocity, dt * config.drag_friction));
    }
}
