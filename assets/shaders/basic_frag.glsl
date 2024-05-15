#version 460 core

in vec3 normal;
in vec2 uv;

out vec4 frag_color;

uniform sampler2D tex;

void main() {
    frag_color = texture(tex, uv);
}

