#ifndef GRAPHICS_MODEL_H
#define GRAPHICS_MODEL_H

#include <string>
#include <vector>

#include "graphics/mesh.h"
#include "graphics/shader.h"

// Represents a model, which is represented by a list of meshes
// Currently only support loading from a OBJ file
struct Model {
    std::vector<Mesh> meshes;

    /**
     * Creates a model from the specified file
     * @param path the path to the model to load
     * @return whether or not loading is successful
     */
    bool create(std::string path);
    void destroy();

    void draw(ShaderProgram const &shader);
};

#endif // GRAPHICS_MODEL_H
