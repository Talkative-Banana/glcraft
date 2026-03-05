#version 330 core

layout (location = 0) in vec2 quadpos;
layout (location = 1) in vec2 aUV;

uniform mat4 uProjUI;

out vec2 uUV;

void main() {
    uUV = aUV;
    gl_Position = uProjUI * vec4(quadpos, 0.0, 1.0);
}
