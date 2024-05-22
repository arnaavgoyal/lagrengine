#ifndef GRAPHICS_VERTEX_H
#define GRAPHICS_VERTEX_H

#include <glm/glm.hpp>

/**
 * Contains all the necessary information in a vertex.
 * This includes its position vector its normal vector, and its uv
 */
struct Vertex {
    /** the position of the vertex */
    glm::vec3 position;
    /** the normal of the vertex */
    glm::vec3 normal;
    /** the uv of the vertex */
    glm::vec2 uv;
};

#endif // GRAPHICS_VERTEX_H
