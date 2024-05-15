#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <windows.h>

#include "graphics/mesh.h"
#include "graphics/model.h"
#include "graphics/shader.h"

struct OpenGLWrapper {

    HDC *dc;
    HGLRC *rc;
    HMODULE opengl;

    int init(HWND *actual, HDC *dc, HGLRC *rc, HINSTANCE inst,
            char const *class_name, char const *title, unsigned width,
            unsigned height);
    int useShaderProgram(ShaderProgram shader);
    void drawMesh(ShaderProgram shader, Mesh &mesh);
    void drawModel(ShaderProgram shader, Model &mesh);
    int initPipeline(unsigned vertices_len, float *vertices);
    int doDrawIteration();
    void destroy() {

        // destroy everything necessary
        wglMakeCurrent(0, 0);
        wglDeleteContext(*rc);
    }

};

#endif
