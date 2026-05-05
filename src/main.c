#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <time.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/extensions/Xrandr.h>
#include <X11/cursorfont.h>

#include "la.h"
#include "config.h"
#include "navigation.h"
#include "screenshot.h"
#include "render.h"

typedef struct {
    bool is_enabled;
    float shadow;
    float radius;
    float delta_radius;
} Flashlight;

#define INITIAL_FL_DELTA_RADIUS 250.0f
#define FL_DELTA_RADIUS_DECELERATION 10.0f

void flashlight_update(Flashlight *flashlight, float dt) {
    if (fabsf(flashlight->delta_radius) > 1.0f) {
        flashlight->radius = fmaxf(0.0f, flashlight->radius + flashlight->delta_radius * dt);
        flashlight->delta_radius -= flashlight->delta_radius * FL_DELTA_RADIUS_DECELERATION * dt;
    }

    if (flashlight->is_enabled) {
        flashlight->shadow = fminf(flashlight->shadow + 6.0f * dt, 0.8f);
    } else {
        flashlight->shadow = fmaxf(flashlight->shadow - 6.0f * dt, 0.0f);
    }
}

Vec2f get_cursor_position(Display *display) {
    Window root, child;
    int root_x, root_y, win_x, win_y;
    unsigned int mask;
    XQueryPointer(display, DefaultRootWindow(display), &root, &child, &root_x, &root_y, &win_x, &win_y, &mask);
    return vec2((float)root_x, (float)root_y);
}

int x_eleven_error_handler(Display *display, XErrorEvent *error_event) {
    char error_message[256];
    XGetErrorText(display, error_event->error_code, error_message, sizeof(error_message));
    fprintf(stderr, "X ELEVEN ERROR: %s\n", error_message);
    return 0;
}

void usage_quit() {
    printf("Usage: boomer [OPTIONS]\n"
           "  -d, --delay <seconds: float>  delay execution of the program by provided <seconds>\n"
           "  -h, --help                    show this help and exit\n"
           "      --new-config [filepath]   generate a new default config at [filepath]\n"
           "  -c, --config <filepath>       use config at <filepath>\n"
           "  -V, --version                 show the current version and exit\n"
           "  -w, --windowed                windowed mode instead of fullscreen\n");
    exit(0);
}

void version_quit() {
    printf("boomer-dfd4e1f5\n");
    exit(0);
}

Window select_window(Display *display) {
    Cursor cursor = XCreateFontCursor(display, XC_crosshair);
    Window root = DefaultRootWindow(display);
    XGrabPointer(display, root, False, ButtonMotionMask | ButtonPressMask | ButtonReleaseMask, GrabModeAsync, GrabModeAsync, root, cursor, CurrentTime);
    XGrabKeyboard(display, root, False, GrabModeAsync, GrabModeAsync, CurrentTime);

    XEvent event;
    Window selected = root;
    while (true) {
        XNextEvent(display, &event);
        if (event.type == ButtonPress) {
            selected = event.xbutton.subwindow;
            if (selected == None) selected = root;
            break;
        } else if (event.type == KeyPress) {
            break;
        }
    }

    XUngrabPointer(display, CurrentTime);
    XUngrabKeyboard(display, CurrentTime);
    XFreeCursor(display, cursor);
    return selected;
}

typedef struct {
    bool quitting;
    bool windowed;
    Display *display;
    RenderContext *render;
    Camera *camera;
    Mouse *mouse;
    Flashlight *flashlight;
    Config config;
    Screenshot *screenshot;
    XWindowAttributes *wa;
    float dt;
} AppState;

void handle_events(AppState *app) {
    while (XPending(app->display) > 0) {
        XEvent xev;
        XNextEvent(app->display, &xev);
        switch (xev.type) {
            case ConfigureNotify:
                app->wa->width = xev.xconfigure.width;
                app->wa->height = xev.xconfigure.height;
                break;
            case MotionNotify:
                app->mouse->curr = vec2((float)xev.xmotion.x, (float)xev.xmotion.y);
                if (app->mouse->drag) {
                    Vec2f delta = vec2_sub(camera_world(*app->camera, app->mouse->prev), camera_world(*app->camera, app->mouse->curr));
                    app->camera->position = vec2_add(app->camera->position, delta);
                    app->camera->velocity = vec2_mul_scalar(delta, 1.0f / app->dt);
                }
                app->mouse->prev = app->mouse->curr;
                break;
            case KeyPress: {
                KeySym key = XLookupKeysym(&xev.xkey, 0);
                if (key == XK_Escape || key == XK_q) app->quitting = true;
                if (key == XK_z) app->flashlight->is_enabled = !app->flashlight->is_enabled;
                if (key == XK_0) {
                    app->camera->scale = 1.0f;
                    app->camera->delta_scale = 0.0f;
                    app->camera->position = vec2(0.0f, 0.0f);
                    app->camera->velocity = vec2(0.0f, 0.0f);
                }
                if (key == XK_equal) {
                    if ((xev.xkey.state & ControlMask) && app->flashlight->is_enabled) {
                        app->flashlight->delta_radius += INITIAL_FL_DELTA_RADIUS;
                    } else {
                        app->camera->delta_scale += app->config.scroll_speed;
                        app->camera->scale_pivot = app->mouse->curr;
                    }
                }
                if (key == XK_minus) {
                    if ((xev.xkey.state & ControlMask) && app->flashlight->is_enabled) {
                        app->flashlight->delta_radius -= INITIAL_FL_DELTA_RADIUS;
                    } else {
                        app->camera->delta_scale -= app->config.scroll_speed;
                        app->camera->scale_pivot = app->mouse->curr;
                    }
                }
                break;
            }
            case ButtonPress:
                if (xev.xbutton.button == Button1) {
                    app->mouse->drag = true;
                    app->camera->velocity = vec2(0.0f, 0.0f);
                }
                if (xev.xbutton.button == Button4) {
                    if ((xev.xbutton.state & ControlMask) && app->flashlight->is_enabled) {
                        app->flashlight->delta_radius -= INITIAL_FL_DELTA_RADIUS;
                    } else {
                        app->camera->delta_scale += app->config.scroll_speed;
                        app->camera->scale_pivot = app->mouse->curr;
                    }
                }
                if (xev.xbutton.button == Button5) {
                    if ((xev.xbutton.state & ControlMask) && app->flashlight->is_enabled) {
                        app->flashlight->delta_radius += INITIAL_FL_DELTA_RADIUS;
                    } else {
                        app->camera->delta_scale -= app->config.scroll_speed;
                        app->camera->scale_pivot = app->mouse->curr;
                    }
                }
                break;
            case ButtonRelease:
                if (xev.xbutton.button == Button1) app->mouse->drag = false;
                break;
        }
    }
}

int main(int argc, char** argv) {
    float delay_sec = 0.0f;
    bool windowed = false;
    char* config_file = NULL;
    bool select_mode = false;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--delay") == 0) {
            if (i + 1 < argc) delay_sec = atof(argv[++i]);
            else usage_quit();
        } else if (strcmp(argv[i], "-w") == 0 || strcmp(argv[i], "--windowed") == 0) {
            windowed = true;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            usage_quit();
        } else if (strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "--version") == 0) {
            version_quit();
        } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--config") == 0) {
            if (i + 1 < argc) config_file = argv[++i];
            else usage_quit();
        } else if (strcmp(argv[i], "--select") == 0) {
            select_mode = true;
        } else if (strcmp(argv[i], "--new-config") == 0) {
            char* new_config_path = "config";
            if (i + 1 < argc && argv[i+1][0] != '-') new_config_path = argv[++i];
            if (access(new_config_path, F_OK) == 0) {
                printf("File %s already exists. Replace it? [yn] ", new_config_path);
                char answer = (char)getchar();
                if (answer != 'y') { printf("Disaster prevented\n"); exit(1); }
            }
            generate_default_config(new_config_path);
            printf("Generated config at %s\n", new_config_path);
            exit(0);
        } else {
            fprintf(stderr, "Unknown flag `%s`\n", argv[i]);
            usage_quit();
        }
    }

    if (delay_sec > 0.0f) usleep((unsigned int)(delay_sec * 1000000.0f));

    Config config = default_config;
    if (config_file && access(config_file, F_OK) == 0) config = load_config(config_file);

    Display *display = XOpenDisplay(NULL);
    if (!display) { fprintf(stderr, "Failed to open display\n"); return 1; }
    XSetErrorHandler(x_eleven_error_handler);

    Window root = DefaultRootWindow(display);
    Window tracking_window = root;
    if (select_mode) {
        printf("Please select window:\n");
        tracking_window = select_window(display);
    }

    XRRScreenConfiguration *screen_config = XRRGetScreenInfo(display, root);
    short rate = XRRConfigCurrentRate(screen_config);
    if (rate <= 0) rate = 60;

    XWindowAttributes root_attrs;
    XGetWindowAttributes(display, root, &root_attrs);

    RenderContext render = render_init(display, root, windowed, root_attrs.width, root_attrs.height);
    Screenshot screenshot = new_screenshot(display, tracking_window);
    render_update_texture(&render, screenshot.image);

    Camera camera = { .scale = 1.0f };
    Vec2f initial_mouse_pos = get_cursor_position(display);
    Mouse mouse = { .curr = initial_mouse_pos, .prev = initial_mouse_pos };
    Flashlight flashlight = { .is_enabled = false, .radius = 200.0f };

    XWindowAttributes wa;
    XGetWindowAttributes(display, render.win, &wa);

    AppState app = {
        .quitting = false,
        .windowed = windowed,
        .display = display,
        .render = &render,
        .camera = &camera,
        .mouse = &mouse,
        .flashlight = &flashlight,
        .config = config,
        .screenshot = &screenshot,
        .wa = &wa,
        .dt = 1.0f / (float)rate
    };

    float dt = 1.0f / (float)rate;
    struct timespec frame_start, frame_end;

    while (!app.quitting) {
        clock_gettime(CLOCK_MONOTONIC, &frame_start);

        if (!windowed) XSetInputFocus(display, render.win, RevertToParent, CurrentTime);

        handle_events(&app);

        camera_update(&camera, config, dt, mouse, screenshot.image, vec2((float)wa.width, (float)wa.height));
        flashlight_update(&flashlight, dt);

        glViewport(0, 0, wa.width, wa.height);
        render_draw(&render, screenshot, camera, vec2((float)wa.width, (float)wa.height), mouse, flashlight.shadow, flashlight.radius);
        glXSwapBuffers(display, render.win);

        clock_gettime(CLOCK_MONOTONIC, &frame_end);
        double elapsed = (frame_end.tv_sec - frame_start.tv_sec) + (frame_end.tv_nsec - frame_start.tv_nsec) / 1000000000.0;
        if (elapsed < dt) {
            struct timespec sleep_time = { .tv_sec = 0, .tv_nsec = (long)((dt - elapsed) * 1000000000.0) };
            nanosleep(&sleep_time, NULL);
        }
    }

    render_free(&render);
    screenshot_destroy(&screenshot, display);
    XCloseDisplay(display);
    return 0;
}
