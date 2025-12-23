#version 330 core

in vec2 uUV;
out vec4 outColor;

uniform sampler2D UICOMP;
void main() {
   vec4 texColor = texture(UICOMP, uUV);

   // Discard fully transparent pixels (optional but recommended)
   if (texColor.a < 0.01)
      discard;

   outColor = texColor; 
}
