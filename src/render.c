#include "render.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glext.h>

// OpenGL function pointers
static PFNGLCREATESHADERPROC glCreateShader;
static PFNGLSHADERSOURCEPROC glShaderSource;
static PFNGLCOMPILESHADERPROC glCompileShader;
static PFNGLGETSHADERIVPROC glGetShaderiv;
static PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
static PFNGLCREATEPROGRAMPROC glCreateProgram;
static PFNGLATTACHSHADERPROC glAttachShader;
static PFNGLLINKPROGRAMPROC glLinkProgram;
static PFNGLDELETESHADERPROC glDeleteShader;
static PFNGLGETPROGRAMIVPROC glGetProgramiv;
static PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
static PFNGLUSEPROGRAMPROC glUseProgram;
static PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
static PFNGLGENBUFFERSPROC glGenBuffers;
static PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
static PFNGLBINDBUFFERPROC glBindBuffer;
static PFNGLBUFFERDATAPROC glBufferData;
static PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
static PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
static PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
static PFNGLDELETEBUFFERSPROC glDeleteBuffers;
static PFNGLUNIFORM1FPROC glUniform1f;
static PFNGLUNIFORM2FPROC glUniform2f;
static PFNGLUNIFORM1IPROC glUniform1i;
static PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
static PFNGLGENERATEMIPMAPPROC glGenerateMipmap;
static PFNGLDELETEPROGRAMPROC glDeleteProgram;

static void load_extensions() {
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

static GLuint new_shader(const char* content, GLenum kind, const char* path) {
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

static GLuint new_shader_program(const char* vertex_content, const char* fragment_content) {
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

RenderContext render_init(Display *display, Window root, bool windowed, int width, int height) {
    RenderContext ctx = {0};
    ctx.display = display;

    int screen = DefaultScreen(display);
    int attrs[] = {
        GLX_RGBA,
        GLX_DEPTH_SIZE, 24,
        GLX_DOUBLEBUFFER,
        None
    };

    XVisualInfo *vi = glXChooseVisual(display, screen, attrs);
    if (!vi) {
        fprintf(stderr, "No appropriate visual found\n");
        exit(1);
    }

    XSetWindowAttributes swa;
    swa.colormap = XCreateColormap(display, root, vi->visual, AllocNone);
    swa.event_mask = ButtonPressMask | ButtonReleaseMask | KeyPressMask | KeyReleaseMask | PointerMotionMask | ExposureMask;
    
    unsigned long mask = CWColormap | CWEventMask;
    if (!windowed) {
        swa.override_redirect = True;
        swa.save_under = True;
        mask |= CWOverrideRedirect | CWSaveUnder;
    }

    ctx.win = XCreateWindow(display, root, 0, 0, width, height, 0, vi->depth, InputOutput, vi->visual, mask, &swa);
    XMapWindow(display, ctx.win);
    XStoreName(display, ctx.win, "boomer");

    ctx.glc = glXCreateContext(display, vi, NULL, GL_TRUE);
    glXMakeCurrent(display, ctx.win, ctx.glc);

    load_extensions();

    ctx.shader_program = new_shader_program(vert_src, frag_src);

    glGenVertexArrays(1, &ctx.vao);
    glGenBuffers(1, &ctx.vbo);
    glGenBuffers(1, &ctx.ebo);

    glGenTextures(1, &ctx.texture);

    return ctx;
}

void render_update_texture(RenderContext *ctx, XImage *image) {
    float w = (float)image->width;
    float h = (float)image->height;
    float vertices[] = {
        w, 0.0f, 0.0f, 1.0f, 1.0f,
        w, h,    0.0f, 1.0f, 0.0f,
        0.0f, h,    0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 1.0f
    };
    unsigned int indices[] = { 0, 1, 3, 1, 2, 3 };

    glBindVertexArray(ctx->vao);
    glBindBuffer(GL_ARRAY_BUFFER, ctx->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctx->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    int stride = 5 * sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ctx->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->width, image->height, 0, GL_BGRA, GL_UNSIGNED_BYTE, image->data);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void render_draw(RenderContext *ctx, Screenshot screenshot, Camera camera, Vec2f window_size, Mouse mouse, float flShadow, float flRadius) {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(ctx->shader_program);

    glUniform2f(glGetUniformLocation(ctx->shader_program, "cameraPos"), camera.position.x, camera.position.y);
    glUniform1f(glGetUniformLocation(ctx->shader_program, "cameraScale"), camera.scale);
    glUniform2f(glGetUniformLocation(ctx->shader_program, "screenshotSize"), (float)screenshot.image->width, (float)screenshot.image->height);
    glUniform2f(glGetUniformLocation(ctx->shader_program, "windowSize"), window_size.x, window_size.y);
    glUniform2f(glGetUniformLocation(ctx->shader_program, "cursorPos"), mouse.curr.x, mouse.curr.y);
    glUniform1f(glGetUniformLocation(ctx->shader_program, "flShadow"), flShadow);
    glUniform1f(glGetUniformLocation(ctx->shader_program, "flRadius"), flRadius);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ctx->texture);
    glUniform1i(glGetUniformLocation(ctx->shader_program, "tex"), 0);

    glBindVertexArray(ctx->vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}

void render_free(RenderContext *ctx) {
    glDeleteProgram(ctx->shader_program);
    glDeleteVertexArrays(1, &ctx->vao);
    glDeleteBuffers(1, &ctx->vbo);
    glDeleteBuffers(1, &ctx->ebo);
    glDeleteTextures(1, &ctx->texture);
    glXDestroyContext(ctx->display, ctx->glc);
    XDestroyWindow(ctx->display, ctx->win);
}
