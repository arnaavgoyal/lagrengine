#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <windows.h>

#include "graphics/shader.h"

struct OpenGLGraphicsAbstractor {

    HDC *dc;

    int init(HDC *dc, HGLRC *rc, HINSTANCE inst);
    int useShaderProgram(ShaderProgram shader);
    int doDrawIteration();

};

#endif
