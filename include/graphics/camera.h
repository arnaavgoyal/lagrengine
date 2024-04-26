#ifndef GRAPHICS_CAMERA_H
#define GRAPHICS_CAMERA_H

#include <glad/gl.h>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

struct Camera {

    unsigned width;
    unsigned height;
    glm::vec3 pos;
    glm::vec3 front;
    glm::vec3 up;
    float fov;

    void create(unsigned vp_width, unsigned vp_height,
        glm::vec3 world_pos, glm::vec3 cam_front,
        glm::vec3 world_up, float fov_deg) {
        
        width = vp_width;
        height = vp_height;
        pos = world_pos;
        front = cam_front;
        up = world_up;
        fov = fov_deg;
    }

};

#endif
