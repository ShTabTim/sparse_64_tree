#version 330 core

uniform sampler2D screenTexture;

out vec2 TexCoords;

void main() {
    vec2 size = textureSize(screenTexture, 0);
    size = size/max(1,max(size.x, size.y));
    TexCoords = vec2((gl_VertexID&2)>>1, gl_VertexID&1);
    gl_Position = vec4((2.0*TexCoords-1.0)*size, 0.0, 1.0);
}