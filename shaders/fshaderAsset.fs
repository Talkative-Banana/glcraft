#version 330 core

in vec3 n;
in vec3 e;
in vec3 l;
in vec2 assetUV;
uniform sampler2D assetTexture;

out vec4 outColor;

void main(void) {
    vec3 N = normalize(n);
    vec3 L = normalize(l);
    vec3 E = normalize(e);
    vec3 base = texture(assetTexture, assetUV).rgb;

    float diffuse = max(dot(N, L), 0.0);
    vec3 H = normalize(L + E);
    float specular = pow(max(dot(N, H), 0.0), 24.0);

    vec3 litColor = base * (0.62 + 0.42 * diffuse) + vec3(0.035) * specular;
    outColor = vec4(litColor, 1.0);
}
