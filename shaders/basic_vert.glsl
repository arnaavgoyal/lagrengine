#version 460 core

layout(location = 0) in vec3 attrib_position;
layout(location = 1) in vec2 attrib_uv;

out vec2 uv;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main() {
    uv = attrib_uv;
    gl_Position = proj * view * model * vec4(attrib_position, 1.0f);
}

