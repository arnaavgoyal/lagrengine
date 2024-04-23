#ifndef GRAPHICS_MESH_H
#define GRAPHICS_MESH_H

#include <vector>

#include <glm/glm.hpp>

#include "vertex.h"

// A basic component of a model
// If we imagine a knight, there would likely be meshes for the head, the body,
// the arms, the legs, the sword, the shield, the helmet, ...
class Mesh {
    // OpenGL objects needed for the mesh
    unsigned int vao, vbo, ebo;
    unsigned int num_indices;

public:
    void create(std::vector<Vertex> vertices,
            std::vector<unsigned int> indices);
    void draw();
};

#endif // GRAPHICS_MESH_H
