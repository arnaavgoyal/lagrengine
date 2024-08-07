#ifndef GRAPHICS_MESH_H
#define GRAPHICS_MESH_H

#include <string>
#include <vector>

#include "graphics/shader.h"
#include "graphics/material.h"
#include "graphics/texture.h"
#include "graphics/vertex.h"

// A basic component of a model
// If we imagine a knight, there would likely be meshes for the head, the body,
// the arms, the legs, the sword, the shield, the helmet, ...
struct Mesh {
    // OpenGL objects needed for the mesh
    unsigned int vao, vbo, ebo;
    unsigned int num_indices;
    std::vector<Material> materials;

    void create(std::vector<Vertex> vertices,
            std::vector<unsigned int> indices,
            std::vector<Material> materials);
    void destroy();

    void draw(ShaderProgram const &shader);
};

#endif // GRAPHICS_MESH_H
