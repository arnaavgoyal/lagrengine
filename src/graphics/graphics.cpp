#include <windows.h>

#include <glad/gl.h>
#include <glad/wgl.h>

#include "graphics/graphics.h"
#include "graphics/mesh.h"
#include "graphics/shader.h"
#include "os/window.h"

/**
 * I hate everything.
 * @param mod the module to load from
 * @param proc_name the procedure to load
 * @return the loaded procedure
 */
static FARPROC getOpenGLFunction(HMODULE mod, char const *proc_name) {

    FARPROC proc = GetProcAddress(mod, proc_name);

    if(proc)
        return proc;

    proc = wglGetProcAddress(proc_name);

    return proc;
}

int OpenGLWrapper::init(HWND *window, HDC *dc, HGLRC *rc, HINSTANCE inst, char const *class_name, char const *title, unsigned width, unsigned height) {

    // create a dummy window
    HWND dummy_window = createWindow(inst, class_name, title, width, height, false);

    // get the device context for the current window
    HDC dummy_dc = GetDC(dummy_window);

    if(!dummy_dc) {
        fprintf(stderr, "Failed to get dummy DC\n");

        // false on failure
        return false;
    }

    // setup the dummy pixel format
    PIXELFORMATDESCRIPTOR dummy_pfd = {
        .nSize = sizeof(PIXELFORMATDESCRIPTOR),
        .nVersion = 1,
        .dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        .iPixelType = PFD_TYPE_RGBA,
        .cColorBits = 32,
        .cDepthBits = 24,
        .cStencilBits = 8,
        .cAuxBuffers = 0,
        .iLayerType = PFD_MAIN_PLANE,
    };

    int dummy_pixel_format = ChoosePixelFormat(dummy_dc, &dummy_pfd);

    if(!dummy_pixel_format) {
        fprintf(stderr, "Failed to choose pixel format for the dummy DC\n");

        // false on failure
        return false;
    }

    if(!SetPixelFormat(dummy_dc, dummy_pixel_format, &dummy_pfd)) {
        fprintf(stderr, "Failed to set pixel format for the dummy DC\n");

        // false on failure
        return false;
    }

    // create a dummy rendering context
    HGLRC dummy_rc = wglCreateContext(dummy_dc);

    if(!dummy_rc) {
        fprintf(stderr, "Failed to create a dummy rendering context\n");

        // false on failure
        return false;
    }

    if(!wglMakeCurrent(dummy_dc, dummy_rc)) {
        fprintf(stderr, "Failed to make the dummy context current\n");

        // false on failure
        return false;
    }

    if(!gladLoadWGL(dummy_dc, (GLADloadfunc) wglGetProcAddress)) {
        fprintf(stderr, "Failed to load WGL functions\n");

        // false on failure
        return false;
    }

    wglMakeCurrent(0, 0);
    wglDeleteContext(dummy_rc);
    DestroyWindow(dummy_window);

    // create a window
    *window = createWindow(inst, class_name, title, width, height, true);

    int pixel_format_attribs[] = {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
        WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
        WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
        WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB, 32,
        WGL_DEPTH_BITS_ARB, 24,
        WGL_STENCIL_BITS_ARB, 8,
        0
    };

    *dc = GetDC(*window);

    int pixel_format;
    unsigned int num_formats;
    wglChoosePixelFormatARB(*dc, pixel_format_attribs, 0, 1, &pixel_format,
            &num_formats);

    if(!num_formats) {
        fprintf(stderr, "Failed to choose the pixel format for the DC\n");

        // false on failure
        return false;
    }

    PIXELFORMATDESCRIPTOR pfd;
    DescribePixelFormat(*dc, pixel_format, sizeof(pfd), &pfd);

    if(!SetPixelFormat(*dc, pixel_format, &pfd)) {
        fprintf(stderr, "Failed to set pixel format for the DC\n");

        // false on failure
        return false;
    }

    int gl_attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 6,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
    };

    *rc = wglCreateContextAttribsARB(*dc, 0, gl_attribs);

    if(!(*rc)) {
        fprintf(stderr, "Failed to create a rendering context\n");

        // false on failure
        return false;
    }

    wglMakeCurrent(*dc, *rc);

    opengl = LoadLibraryA("opengl32.dll");

    if(!opengl) {
        fprintf(stderr, "Failed to load OpenGL32.dll\n");

        return false;
    }

    if(!gladLoadGLUserPtr((GLADuserptrloadfunc) getOpenGLFunction, opengl)) {
        fprintf(stderr, "Failed to initialize GLAD\n");

        // false on failure
        return false;
    }

    // set the viewport to the client window size
    glViewport(0, 0, width, height);

    // set the clear color for the context
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // set class members
    this->dc = dc;
    this->rc = rc;

    // true on success
    return true;
}

int OpenGLWrapper::useShaderProgram(ShaderProgram program) {
    glUseProgram(program);
    return 1;
}

void OpenGLWrapper::drawMesh(ShaderProgram program, Mesh &mesh) {
    for(TextureUniform tu : mesh.texture_uniforms) {
        glUniform1i(glGetUniformLocation(program, tu.name.c_str()), tu.value);
    }
    mesh.draw();
}

int OpenGLWrapper::initPipeline(unsigned vertices_len, float *vertices) {

    // very basic VAO and VBO implementation
    unsigned int vao, vbo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices_len, vertices,
            GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
            (void*) 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
            (void*) (3 * sizeof(float)));

    return 1;
}

int OpenGLWrapper::doDrawIteration() {

    // clear the buffer
	glClear(GL_COLOR_BUFFER_BIT);

    // draw a triangle
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // swap buffers
    SwapBuffers(*dc);

    return 1;
}
