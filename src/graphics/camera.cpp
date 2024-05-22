#include <glm/gtc/type_ptr.hpp>

#include "graphics/camera.h"

void Camera::init(glm::vec3 world_pos, glm::vec3 cam_front, glm::vec3 world_up,
        float fov, float aspect_ratio) {
    pos = world_pos;
    front = cam_front;
    up = world_up;
    setProjection(fov, aspect_ratio);
}

void Camera::setProjection(float fov, float aspect_ratio) {
    proj = glm::perspective(glm::radians(fov), aspect_ratio, 0.1f, 100.0f);
}

glm::mat4 Camera::getView() {
    return glm::lookAt(pos, pos + front, up);
}

