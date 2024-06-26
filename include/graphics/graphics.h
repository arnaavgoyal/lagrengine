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

    /**
     * Initializes the graphics engine
     * @param inst the current instance
     * @param class_name the name of the window class to use
     * @param title the title of the window
     * @param width the width of the window in pixels
     * @param height the height of the window in pixels
     * @return whether or not the initialization suceeds
     */
    bool init(HINSTANCE inst, char const *class_name, char const *title,
            unsigned width, unsigned height);

    /**
     * Destroys the graphics engine
     */
    void destroy();
};

#endif // GRAPHICS_GRAPHICS_H
