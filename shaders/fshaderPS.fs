#version 330 core

in vec2 uUV;
in float ualpha;
// in vec2 utexCord;
out vec4 outColor;

uniform sampler2D UICOMP;
void main() {
   vec4 texColor = texture(UICOMP, uUV);
   texColor.a *= ualpha;

   // Discard fully transparent pixels
   if (texColor.a < 0.01)
      discard;

   outColor = texColor; 
}
