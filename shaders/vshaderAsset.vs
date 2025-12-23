#version 330 core

in vec3 vVertex;   // position in model space
in vec3 vNormal;   // normal in model space

uniform mat4 vModel;
uniform mat4 vView;
uniform mat4 vProjection;
uniform vec3 lightpos;   // light position in world space
uniform vec3 cameraPos;  // camera position in world space

out vec3 n;  // normal in world space
out vec3 e;  // eye vector in world space
out vec3 l;  // light vector in world space

void main() {
    // final clip-space position
    gl_Position = vProjection * vView * vModel * vec4(vVertex, 1.0);

    // position in world space
    vec3 lightPos = vec3(vModel * vec4(vVertex, 1.0));

    // transform normal to world space
    mat3 normalMatrix = transpose(inverse(mat3(vModel)));
    n = normalize(normalMatrix * vNormal);

    // light direction (world space)
    l = normalize(lightpos - lightPos);

    // eye direction (world space)
    e = normalize(cameraPos - lightPos);
}
