#include <fstream>
#include <iostream>
#include <stdio.h>

#include <glad/gl.h>

#include "graphics/shader.h"

bool ShaderProgram::create(std::string vertex_path, std::string fragment_path) {
    std::ifstream vertex_file(vertex_path);

    if(vertex_file.fail()) {
        fprintf(stderr, "Vertex shader %s does not exist\n",
            vertex_path.c_str());

        // false on failure
        return false;
    }

    std::string vertex_str = std::string(
        std::istreambuf_iterator<char>(vertex_file),
        std::istreambuf_iterator<char>());
    vertex_file.close();

    // read the fragment file into a string
    std::ifstream fragment_file(fragment_path.c_str());

    if(fragment_file.fail()) {
        fprintf(stderr, "Fragment shader %s does not exist\n",
            fragment_path.c_str());

        // false on failure
        return false;
    }

    std::string fragment_str = std::string(
        std::istreambuf_iterator<char>(fragment_file),
        std::istreambuf_iterator<char>());
    fragment_file.close();

    // error variables
    int status;
    char infoLog[1024];

    // initialize the vertex shader
    unsigned int vertex_id = glCreateShader(GL_VERTEX_SHADER);

    char const *vertex_cstr = vertex_str.c_str();
    glShaderSource(vertex_id, 1, &vertex_cstr, 0);
    glCompileShader(vertex_id);
    glGetShaderiv(vertex_id, GL_COMPILE_STATUS, &status);

    if(!status) {
        glGetShaderInfoLog(vertex_id, 1024, 0, infoLog);
        fprintf(stderr, "Failed to compile vertex shader: %s\n",
            vertex_path.c_str());
        fprintf(stderr, "%s\n", infoLog);

        // clean up
        glDeleteShader(vertex_id);

        // return false on failure
        return false;
    }

    // initialize the fragment shader
    unsigned int fragment_id = glCreateShader(GL_FRAGMENT_SHADER);

    char const *fragment_cstr = fragment_str.c_str();
    glShaderSource(fragment_id, 1, &fragment_cstr, 0);
    glCompileShader(fragment_id);
    glGetShaderiv(fragment_id, GL_COMPILE_STATUS, &status);

    if(!status) {
        glGetShaderInfoLog(fragment_id, 1024, 0, infoLog);
        fprintf(stderr, "Failed to compile fragment shader: %s\n",
            fragment_path.c_str());
        fprintf(stderr, "%s\n", infoLog);

        // clean up
        glDeleteShader(vertex_id);
        glDeleteShader(fragment_id);

        // return false on failure
        return false;
    }

    // create and link the shader program
    id = glCreateProgram();
    glAttachShader(id, vertex_id);
    glAttachShader(id, fragment_id);
    glLinkProgram(id);

    glGetProgramiv(id, GL_COMPILE_STATUS, &status);

    if(!status) {
        glGetProgramInfoLog(id, 1024, 0, infoLog);
        fprintf(stderr, "Failed to link shader program\n");
        fprintf(stderr, "%s\n", infoLog);

        // clean up
        glDeleteShader(vertex_id);
        glDeleteShader(fragment_id);
        glDeleteProgram(id);

        // return false on failure
        return false;
    }

    // delete shaders, they are no longer needed
    glDeleteShader(vertex_id);
    glDeleteShader(fragment_id);

    return true;
}

void ShaderProgram::destroy() {
    glDeleteProgram(id);
}

bool ShaderProgram::setUniformInt(std::string name, int value) const {
    int location = glGetUniformLocation(id, name.c_str());
    if(location == -1) {
        return false;
    }

    glUniform1i(location, value);

    return true;
}

bool ShaderProgram::setUniformFloat(std::string name, float value) const {
    int location = glGetUniformLocation(id, name.c_str());
    if(location == -1) {
        return false;
    }

    glUniform1f(location, value);

    return true;
}

bool ShaderProgram::operator<(ShaderProgram const other) const {
    return this->id < other.id;
}
