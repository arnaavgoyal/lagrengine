#ifndef GRAPHICS_VERTEX_H
#define GRAPHICS_VERTEX_H

#include <glm/glm.hpp>

// Contains all the necessary information in a vertex.
// Contains its position vector, uv coords, and more stuff later
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
};

#endif // GRAPHICS_VERTEX_H
