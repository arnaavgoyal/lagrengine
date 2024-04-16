#ifndef UTILS_SHADER_H
#define UTILS_SHADER_H

#include <string>

using ShaderProgram = unsigned int;

/**
 * Compiles and returns a basic shader program.
 * @param vertex_path path to vertex shader
 * @param fragment_path path to fragment shader
 * @return the newly compiled shader program
 */
ShaderProgram compileShaderProgram(std::string vertex_path, 
        std::string fragment_path);

#endif // UTILS_SHADER_H
