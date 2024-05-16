#ifndef GRAPHICS_CAMERA_H
#define GRAPHICS_CAMERA_H

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

/**
 * Represents a camera that can be a scene can be rendered from
 */
struct Camera {
    /** the position of the camera */
    glm::vec3 pos;
    /** the camera's front vector */
    glm::vec3 front;
    /** the up vector of the world */
    glm::vec3 up;
    /** the camera's projection matrix */
    glm::mat4 proj;

    /**
     * Initializes the camera
     * @param world_pos the camera's position
     * @param cam_front the front vector of the camera
     * @param vec3 world_up the up vector of the world
     * @param fov the field of view in degrees
     * @param aspect_ratio the window's aspect ratio
     */
    void init(glm::vec3 world_pos, glm::vec3 cam_front, glm::vec3 world_up,
            float fov, float aspect_ratio);
    void setProjection(float fov, float aspect_ratio);
};

#endif // GRAPHICS_CAMERA_HH
