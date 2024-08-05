#ifndef GRAPHICS_GRAPHICS_H
#define GRAPHICS_GRAPHICS_H

#include <windows.h>

#include "graphics/mesh.h"
#include "graphics/model.h"
#include "graphics/shader.h"

/**
 * Wrapper and engine for using OpenGL on Windows
 */
struct OpenGLWrapper {

    /** the window */
    HWND window;
    /** the device context */
    HDC dc;
    /** the rendering context */
    HGLRC rc;

    unsigned width;
    unsigned height;

    /**
     * Initializes the actual window for rendering.
     * Call this on the WindowCallback thread (this function creates the message queue)
    */
    bool init(HINSTANCE inst, char const *class_name, char const *title,
            unsigned wnd_width, unsigned wnd_height);

    /**
     * Initializes rendering stuff and loads OpenGL.
     * Call this on the Rendering thread (OpenGL is only valid on one thread)
    */
    bool initGL();

    /**
     * Destroys the graphics engine
     */
    void destroy();

    void swapBuffers() { SwapBuffers(dc); }
    void captureMouse() { SetCapture(window); }
};

#endif // GRAPHICS_GRAPHICS_H
