#version 330 core

layout(location = 0) in vec3 vVertex;   // position in model space
layout(location = 1) in vec3 vNormal;   // normal in model space

uniform mat4 vModel;
uniform mat4 vView;
uniform mat4 vProjection;

out vec3 n;  // normal in world space
out vec3 e;  // eye vector in world space
out vec3 l;  // light vector in world space

void main() {
    // final clip-space position
    gl_Position = vProjection * vView * vModel * vec4(vVertex, 1.0);

    // position in camera space
    vec3 lightPos = vec3(vModel * vec4(vVertex, 1.0));

    // transform normal to world space
    mat3 normalMatrix = transpose(inverse(mat3(vModel)));
    n = normalize(normalMatrix * vNormal);

    // light direction (world space)
    l = normalize(-lightPos);

    // eye direction (world space)
    e = normalize(-lightPos);
}
