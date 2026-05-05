#ifndef RENDER_H_
#define RENDER_H_

#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include "la.h"
#include "navigation.h"
#include "screenshot.h"

typedef struct {
    Display *display;
    Window win;
    GLXContext glc;
    GLuint shader_program;
    GLuint vao, vbo, ebo;
    GLuint texture;
} RenderContext;

RenderContext render_init(Display *display, Window root, bool windowed, int width, int height);
void render_update_texture(RenderContext *ctx, XImage *image);
void render_draw(RenderContext *ctx, Screenshot screenshot, Camera camera, Vec2f window_size, Mouse mouse, float flShadow, float flRadius);
void render_free(RenderContext *ctx);

#endif // RENDER_H_
