#version 330 core

uniform vec2 position;
uniform vec2 window;

const vec2 delays[3] = vec2[](
    vec2(0, 0),
    vec2(18, -7),
    vec2(8, -18)
);

void main() {
    gl_Position = vec4(2.0*(position + delays[gl_VertexID])/window-1.0, 0.0, 1.0);
}