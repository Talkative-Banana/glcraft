#version 330 core

layout (location = 0) in vec2 quadpos;
layout (location = 1) in vec2 aUV;
layout (location = 2) in float aalpha;

uniform mat4 uProjPS;

out vec2 uUV;
out float ualpha;
// out vec2 utexCord;

void main() {
    uUV = aUV;
	ualpha = aalpha;
	// utexCord = atexCord;
    gl_Position = uProjPS * vec4(quadpos, 0.0, 1.0);
}
