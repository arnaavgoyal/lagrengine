#ifndef UTILS_SHADER_H
#define UTILS_SHADER_H

#include <string>

/**
 * Represents an OpenGL shader program
 */
struct ShaderProgram {
    /** The ID of the program */
    unsigned int id;

    /**
     * Compiles a basic shader program
     * @param vertex_path path to vertex shader
     * @param fragment_path path to fragment shader
     * @return whether or not the creation was successful
     */
    bool create(std::string vertex_path, std::string fragment_path);

    /**
     * Destroys the shader program
     */
    void destroy();

    /**
     * Sets an integer uniform in the given shader program. The shader must be
     * bound for this to work properly
     * @param name the name of the uniform
     * @param value the value to set the uniform too
     * @return whether or not the function succeeded
     */
    bool setUniformInt(std::string name, int value);
};

#endif // UTILS_SHADER_H
