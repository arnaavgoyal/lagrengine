#ifndef GRAPHICS_VERTEX_H
#define GRAPHICS_VERTEX_H

#include <glm/glm.hpp>

// Contains all the necessary information in a vertex.
// Contains its position vector, uv coords, and more stuff later
struct Vertex {
    float position[3];
    float uv_coords[2];
};

#endif // GRAPHICS_VERTEX_H
