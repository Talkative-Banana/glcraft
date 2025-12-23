#version 330 core

in vec3 n; // interpolated normal (world space)
in vec3 e; // eye vector (camera - position, normalized in VS)
in vec3 l; // light vector (lightPos - position, normalized in VS)

out vec4 outColor;

// Light components
vec3 Ls = vec3(1.0, 1.0, 1.0);  // specular light
vec3 Ld = vec3(0.7, 0.7, 0.7);  // diffuse light
vec3 La = vec3(0.0, 0.1, 1.0);  // ambient light

// Material components
vec3 ks = vec3(1.0, 1.0, 1.0);  // specular reflectivity
vec3 kd = vec3(0.5, 0.6, 0.4);  // diffuse reflectivity
vec3 ka = vec3(1.0, 1.0, 1.0);  // ambient reflectivity

float spec_exp = 32.0; // shininess

void main(void) {
    vec3 N = normalize(n);
    vec3 L = normalize(l);
    vec3 E = normalize(e);

    // ambient
    vec3 Ia = ka * La;

    // diffuse
    float diff = max(dot(N, L), 0.0);
    vec3 Id = kd * Ld * diff;

    // specular (Blinn–Phong)
    vec3 H = normalize(L + E);
    float spec = pow(max(dot(N, H), 0.0), spec_exp);
    vec3 Is = ks * Ls * spec;

    // final color
    vec3 fColor = Ia + Id + Is;
    outColor = vec4(fColor, 1.0);
}
