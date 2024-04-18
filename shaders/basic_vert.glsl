#version 460 core

layout(location = 0) in vec3 attrib_position;
layout(location = 1) in vec3 attrib_color;

out vec3 color;

void main() {
    color = attrib_color;
    gl_Position = vec4(attrib_position, 1.0f);
}

