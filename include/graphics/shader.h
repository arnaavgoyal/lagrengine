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
     * Uses the shader program
     */
    void use();

    /**
     * Sets an integer uniform in the given shader program. The shader must be
     * bound for this to work properly
     * @param name the name of the uniform
     * @param value the value to set the uniform too
     * @return whether or not the function succeeded
     */
    bool setUniformInt(std::string name, int value) const;

    /**
     * Sets an float uniform in the given shader program. The shader must be
     * bound for this to work properly
     * @param name the name of the uniform
     * @param value the value to set the uniform too
     * @return whether or not the function succeeded
     */
    bool setUniformFloat(std::string name, float value) const;

    /**
     * A less than operator for maps
     * @param other the other shader program to compare to
     * @return whether or not this shader program is less than the other
     */
    bool operator<(ShaderProgram const other) const;
};

#endif // UTILS_SHADER_H
