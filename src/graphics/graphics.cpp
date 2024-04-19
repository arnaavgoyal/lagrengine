#include <windows.h>

#include <glad/gl.h>
#include <glad/wgl.h>

#include "graphics/graphics.h"
#include "graphics/shader.h"

int OpenGLGraphicsAbstractor::useShaderProgram(ShaderProgram program) {
    glUseProgram(program);
}

int OpenGLGraphicsAbstractor::doDrawIteration() {
    // clear the buffer
	glClear(GL_COLOR_BUFFER_BIT);

    // draw a triangle
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // swap buffers
    SwapBuffers(*dc);
}