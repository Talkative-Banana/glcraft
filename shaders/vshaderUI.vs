#version 330 core

layout (location = 0) in vec2 quadpos;
layout (location = 1) in vec2 aUV;

uniform mat4 uProj;

out vec2 uUV;

void main() {
    uUV = aUV;
    gl_Position = uProj * vec4(quadpos, 0.0, 1.0);
}
