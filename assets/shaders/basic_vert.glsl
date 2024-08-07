#version 460 core

layout(location = 0) in vec3 attrib_position;
layout(location = 1) in vec3 attrib_normal;
layout(location = 2) in vec2 attrib_uv;

out vec3 position;
out vec3 normal;
out vec2 uv;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main() {
    normal = attrib_normal;
    uv = attrib_uv;
    gl_Position = proj * view * model * vec4(attrib_position, 1.0f);
    position = vec3(gl_Position);
}

