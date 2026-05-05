#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/extensions/Xrandr.h>
#include <X11/cursorfont.h>

#include <GL/gl.h>
#include <GL/glx.h>

#include "la.h"
#include "config.h"
#include "navigation.h"
#include "screenshot.h"

// OpenGL function pointers (for modern GL)
#include <GL/glext.h>

PFNGLCREATESHADERPROC glCreateShader;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLDELETESHADERPROC glDeleteShader;
PFNGLGETPROGRAMIVPROC glGetProgramiv;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
PFNGLDELETEBUFFERSPROC glDeleteBuffers;
PFNGLUNIFORM1FPROC glUniform1f;
PFNGLUNIFORM2FPROC glUniform2f;
PFNGLUNIFORM1IPROC glUniform1i;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
PFNGLGENERATEMIPMAPPROC glGenerateMipmap;
PFNGLDELETEPROGRAMPROC glDeleteProgram;

void load_extensions() {
    glCreateShader = (PFNGLCREATESHADERPROC)glXGetProcAddress((const GLubyte*)"glCreateShader");
    glShaderSource = (PFNGLSHADERSOURCEPROC)glXGetProcAddress((const GLubyte*)"glShaderSource");
    glCompileShader = (PFNGLCOMPILESHADERPROC)glXGetProcAddress((const GLubyte*)"glCompileShader");
    glGetShaderiv = (PFNGLGETSHADERIVPROC)glXGetProcAddress((const GLubyte*)"glGetShaderiv");
    glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)glXGetProcAddress((const GLubyte*)"glGetShaderInfoLog");
    glCreateProgram = (PFNGLCREATEPROGRAMPROC)glXGetProcAddress((const GLubyte*)"glCreateProgram");
    glAttachShader = (PFNGLATTACHSHADERPROC)glXGetProcAddress((const GLubyte*)"glAttachShader");
    glLinkProgram = (PFNGLLINKPROGRAMPROC)glXGetProcAddress((const GLubyte*)"glLinkProgram");
    glDeleteShader = (PFNGLDELETESHADERPROC)glXGetProcAddress((const GLubyte*)"glDeleteShader");
    glGetProgramiv = (PFNGLGETPROGRAMIVPROC)glXGetProcAddress((const GLubyte*)"glGetProgramiv");
    glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)glXGetProcAddress((const GLubyte*)"glGetProgramInfoLog");
    glUseProgram = (PFNGLUSEPROGRAMPROC)glXGetProcAddress((const GLubyte*)"glUseProgram");
    glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)glXGetProcAddress((const GLubyte*)"glGenVertexArrays");
    glGenBuffers = (PFNGLGENBUFFERSPROC)glXGetProcAddress((const GLubyte*)"glGenBuffers");
    glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)glXGetProcAddress((const GLubyte*)"glBindVertexArray");
    glBindBuffer = (PFNGLBINDBUFFERPROC)glXGetProcAddress((const GLubyte*)"glBindBuffer");
    glBufferData = (PFNGLBUFFERDATAPROC)glXGetProcAddress((const GLubyte*)"glBufferData");
    glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)glXGetProcAddress((const GLubyte*)"glVertexAttribPointer");
    glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)glXGetProcAddress((const GLubyte*)"glEnableVertexAttribArray");
    glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)glXGetProcAddress((const GLubyte*)"glDeleteVertexArrays");
    glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)glXGetProcAddress((const GLubyte*)"glDeleteBuffers");
    glUniform1f = (PFNGLUNIFORM1FPROC)glXGetProcAddress((const GLubyte*)"glUniform1f");
    glUniform2f = (PFNGLUNIFORM2FPROC)glXGetProcAddress((const GLubyte*)"glUniform2f");
    glUniform1i = (PFNGLUNIFORM1IPROC)glXGetProcAddress((const GLubyte*)"glUniform1i");
    glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)glXGetProcAddress((const GLubyte*)"glGetUniformLocation");
    glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)glXGetProcAddress((const GLubyte*)"glGenerateMipmap");
    glDeleteProgram = (PFNGLDELETEPROGRAMPROC)glXGetProcAddress((const GLubyte*)"glDeleteProgram");
}

GLuint new_shader(const char* content, GLenum kind, const char* path) {
    GLuint shader = glCreateShader(kind);
    glShaderSource(shader, 1, &content, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        fprintf(stderr, "Error during shader compilation: %s. Log:\n%s\n", path, info_log);
    }
    return shader;
}

GLuint new_shader_program(const char* vertex_content, const char* fragment_content) {
    GLuint program = glCreateProgram();
    GLuint vertex_shader = new_shader(vertex_content, GL_VERTEX_SHADER, "vert.glsl");
    GLuint fragment_shader = new_shader(fragment_content, GL_FRAGMENT_SHADER, "frag.glsl");

    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetProgramInfoLog(program, 512, NULL, info_log);
        fprintf(stderr, "Error linking program:\n%s\n", info_log);
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    return program;
}

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

void draw(Screenshot screenshot, Camera camera, GLuint shader, GLuint vao, GLuint texture, Vec2f window_size, Mouse mouse, Flashlight flashlight) {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shader);

    glUniform2f(glGetUniformLocation(shader, "cameraPos"), camera.position.x, camera.position.y);
    glUniform1f(glGetUniformLocation(shader, "cameraScale"), camera.scale);
    glUniform2f(glGetUniformLocation(shader, "screenshotSize"), (float)screenshot.image->width, (float)screenshot.image->height);
    glUniform2f(glGetUniformLocation(shader, "windowSize"), window_size.x, window_size.y);
    glUniform2f(glGetUniformLocation(shader, "cursorPos"), mouse.curr.x, mouse.curr.y);
    glUniform1f(glGetUniformLocation(shader, "flShadow"), flashlight.shadow);
    glUniform1f(glGetUniformLocation(shader, "flRadius"), flashlight.radius);

    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
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

const char* vert_src = "#version 130\n"
"in vec3 aPos;\n"
"in vec2 aTexCoord;\n"
"out vec2 texcoord;\n"
"\n"
"uniform vec2 cameraPos;\n"
"uniform float cameraScale;\n"
"uniform vec2 windowSize;\n"
"uniform vec2 screenshotSize;\n"
"uniform vec2 cursorPos;\n"
"\n"
"vec3 to_world(vec3 v) {\n"
"    vec2 ratio = vec2(\n"
"        windowSize.x / screenshotSize.x / cameraScale,\n"
"        windowSize.y / screenshotSize.y / cameraScale);\n"
"    return vec3((v.x / screenshotSize.x * 2.0 - 1.0) / ratio.x,\n"
"                (v.y / screenshotSize.y * 2.0 - 1.0) / ratio.y,\n"
"                v.z);\n"
"}\n"
"\n"
"void main()\n"
"{\n"
"	gl_Position = vec4(to_world((aPos - vec3(cameraPos * vec2(1.0, -1.0), 0.0))), 1.0);\n"
"	texcoord = aTexCoord;\n"
"}\n";

const char* frag_src = "#version 130\n"
"out mediump vec4 color;\n"
"in mediump vec2 texcoord;\n"
"uniform sampler2D tex;\n"
"uniform vec2 cursorPos;\n"
"uniform vec2 windowSize;\n"
"uniform float flShadow;\n"
"uniform float flRadius;\n"
"uniform float cameraScale;\n"
"\n"
"void main()\n"
"{\n"
"    vec4 cursor = vec4(cursorPos.x, windowSize.y - cursorPos.y, 0.0, 1.0);\n"
"    color = mix(\n"
"        texture(tex, texcoord), vec4(0.0, 0.0, 0.0, 0.0),\n"
"        length(cursor - gl_FragCoord) < (flRadius * cameraScale) ? 0.0 : flShadow);\n"
"}\n";

void version_quit() {
    printf("boomer-dfd4e1f5\n");
    exit(0);
}

int main(int argc, char** argv) {
    float delay_sec = 0.0f;
    bool windowed = false;
    char* config_file = NULL;
    bool select_mode = false;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--delay") == 0) {
            if (i + 1 < argc) {
                delay_sec = atof(argv[++i]);
            } else {
                usage_quit();
            }
        } else if (strcmp(argv[i], "-w") == 0 || strcmp(argv[i], "--windowed") == 0) {
            windowed = true;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            usage_quit();
        } else if (strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "--version") == 0) {
            version_quit();
        } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--config") == 0) {
             if (i + 1 < argc) {
                config_file = argv[++i];
            } else {
                usage_quit();
            }
        } else if (strcmp(argv[i], "--select") == 0) {
            select_mode = true;
        } else if (strcmp(argv[i], "--new-config") == 0) {
            char* new_config_path = "config";
            if (i + 1 < argc && argv[i+1][0] != '-') {
                new_config_path = argv[++i];
            }
            if (access(new_config_path, F_OK) == 0) {
                printf("File %s already exists. Replace it? [yn] ", new_config_path);
                char answer = (char)getchar();
                if (answer != 'y') {
                    printf("Disaster prevented\n");
                    exit(1);
                }
            }
            generate_default_config(new_config_path);
            printf("Generated config at %s\n", new_config_path);
            exit(0);
        } else {
            fprintf(stderr, "Unknown flag `%s`\n", argv[i]);
            usage_quit();
        }
    }

    if (delay_sec > 0.0f) {
        usleep((unsigned int)(delay_sec * 1000000.0f));
    }

    Config config = default_config;
    if (config_file && access(config_file, F_OK) == 0) {
        config = load_config(config_file);
    }

    Display *display = XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "Failed to open display\n");
        return 1;
    }

    XSetErrorHandler(x_eleven_error_handler);

    Window root = DefaultRootWindow(display);
    Window tracking_window = root;
    if (select_mode) {
        printf("Please select window:\n");
        tracking_window = select_window(display);
    }

    XRRScreenConfiguration *screen_config = XRRGetScreenInfo(display, root);
    short rate = XRRConfigCurrentRate(screen_config);

    int screen = DefaultScreen(display);
    int glx_major, glx_minor;
    if (!glXQueryVersion(display, &glx_major, &glx_minor) || (glx_major == 1 && glx_minor < 3) || (glx_major < 1)) {
        fprintf(stderr, "Invalid GLX version\n");
        return 1;
    }

    int attrs[] = {
        GLX_RGBA,
        GLX_DEPTH_SIZE, 24,
        GLX_DOUBLEBUFFER,
        None
    };

    XVisualInfo *vi = glXChooseVisual(display, screen, attrs);
    if (!vi) {
        fprintf(stderr, "No appropriate visual found\n");
        return 1;
    }

    XSetWindowAttributes swa;
    swa.colormap = XCreateColormap(display, root, vi->visual, AllocNone);
    swa.event_mask = ButtonPressMask | ButtonReleaseMask | KeyPressMask | KeyReleaseMask | PointerMotionMask | ExposureMask;
    if (!windowed) {
        swa.override_redirect = True;
        swa.save_under = True;
    }

    XWindowAttributes root_attrs;
    XGetWindowAttributes(display, root, &root_attrs);

    Window win = XCreateWindow(display, root, 0, 0, root_attrs.width, root_attrs.height, 0, vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask | (windowed ? 0 : (CWOverrideRedirect | CWSaveUnder)), &swa);

    XMapWindow(display, win);
    XStoreName(display, win, "boomer");

    GLXContext glc = glXCreateContext(display, vi, NULL, GL_TRUE);
    glXMakeCurrent(display, win, glc);

    load_extensions();

    GLuint shader_program = new_shader_program(vert_src, frag_src);

    Screenshot screenshot = new_screenshot(display, tracking_window);

    float w = (float)screenshot.image->width;
    float h = (float)screenshot.image->height;
    float vertices[] = {
        w, 0.0f, 0.0f, 1.0f, 1.0f,
        w, h,    0.0f, 1.0f, 0.0f,
        0.0f, h,    0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 1.0f
    };
    unsigned int indices[] = { 0, 1, 3, 1, 2, 3 };

    GLuint vao, vbo, ebo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    int stride = 5 * sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    GLuint texture;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenshot.image->width, screenshot.image->height, 0, GL_BGRA, GL_UNSIGNED_BYTE, screenshot.image->data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glUniform1i(glGetUniformLocation(shader_program, "tex"), 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    bool quitting = false;
    Camera camera = { .scale = 1.0f };
    Vec2f initial_mouse_pos = get_cursor_position(display);
    Mouse mouse = { .curr = initial_mouse_pos, .prev = initial_mouse_pos };
    Flashlight flashlight = { .is_enabled = false, .radius = 200.0f };

    float dt = 1.0f / (float)rate;

    while (!quitting) {
        if (!windowed) {
            XSetInputFocus(display, win, RevertToParent, CurrentTime);
        }

        XWindowAttributes wa;
        XGetWindowAttributes(display, win, &wa);
        glViewport(0, 0, wa.width, wa.height);

        while (XPending(display) > 0) {
            XEvent xev;
            XNextEvent(display, &xev);
            switch (xev.type) {
                case MotionNotify:
                    mouse.curr = vec2((float)xev.xmotion.x, (float)xev.xmotion.y);
                    if (mouse.drag) {
                        Vec2f delta = vec2_sub(camera_world(camera, mouse.prev), camera_world(camera, mouse.curr));
                        camera.position = vec2_add(camera.position, delta);
                        camera.velocity = vec2_mul_scalar(delta, (float)rate);
                    }
                    mouse.prev = mouse.curr;
                    break;
                case KeyPress: {
                    KeySym key = XLookupKeysym(&xev.xkey, 0);
                    if (key == XK_Escape || key == XK_q) quitting = true;
                    if (key == XK_f) flashlight.is_enabled = !flashlight.is_enabled;
                    if (key == XK_0) {
                        camera.scale = 1.0f;
                        camera.delta_scale = 0.0f;
                        camera.position = vec2(0.0f, 0.0f);
                        camera.velocity = vec2(0.0f, 0.0f);
                    }
                    if (key == XK_equal) {
                        if ((xev.xkey.state & ControlMask) && flashlight.is_enabled) {
                            flashlight.delta_radius += INITIAL_FL_DELTA_RADIUS;
                        } else {
                            camera.delta_scale += config.scroll_speed;
                            camera.scale_pivot = mouse.curr;
                        }
                    }
                    if (key == XK_minus) {
                        if ((xev.xkey.state & ControlMask) && flashlight.is_enabled) {
                            flashlight.delta_radius -= INITIAL_FL_DELTA_RADIUS;
                        } else {
                            camera.delta_scale -= config.scroll_speed;
                            camera.scale_pivot = mouse.curr;
                        }
                    }
                    break;
                }
                case ButtonPress:
                    if (xev.xbutton.button == Button1) {
                        mouse.drag = true;
                        camera.velocity = vec2(0.0f, 0.0f);
                    }
                    if (xev.xbutton.button == Button4) {
                        if ((xev.xbutton.state & ControlMask) && flashlight.is_enabled) {
                            flashlight.delta_radius -= INITIAL_FL_DELTA_RADIUS;
                        } else {
                            camera.delta_scale += config.scroll_speed;
                            camera.scale_pivot = mouse.curr;
                        }
                    }
                    if (xev.xbutton.button == Button5) {
                        if ((xev.xbutton.state & ControlMask) && flashlight.is_enabled) {
                            flashlight.delta_radius += INITIAL_FL_DELTA_RADIUS;
                        } else {
                            camera.delta_scale -= config.scroll_speed;
                            camera.scale_pivot = mouse.curr;
                        }
                    }
                    break;
                case ButtonRelease:
                    if (xev.xbutton.button == Button1) mouse.drag = false;
                    break;
            }
        }

        camera_update(&camera, config, dt, mouse, screenshot.image, vec2((float)wa.width, (float)wa.height));
        flashlight_update(&flashlight, dt);

        draw(screenshot, camera, shader_program, vao, texture, vec2((float)wa.width, (float)wa.height), mouse, flashlight);

        glXSwapBuffers(display, win);
        glFinish();
    }

    screenshot_destroy(&screenshot, display);
    XCloseDisplay(display);
    return 0;
}
