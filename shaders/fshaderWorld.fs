#version 330 core

in vec2 tile;
in float aoFactor;
in vec3 NormalDir;
in float isoscale;
in vec2 TexCoord;
in float visibility;
out vec4 outColor;

float dark;
uniform sampler2D atlas;
uniform vec3 skyColor;

void main() {
   int atlasSize = 16;                // 16x16 grid
   vec2 tileSize = 1.0 / vec2(atlasSize, atlasSize);
   vec2 atlasUV = (TexCoord + vec2(tile.x, tile.y)) * tileSize;

   // outColor = isoscale * texture(atlas, atlasUV);

   vec4 texColor = texture(atlas, atlasUV);
   // if (texColor.a < 0.1) discard;
   if(tile.x == 15 && tile.y == 15) {
      dark = 1.0;
   } else {
      dark = 0.4;
   }
   outColor = vec4(texColor.rgb * isoscale * aoFactor * dark, texColor.a);
   outColor = mix(vec4(skyColor, 1.0), outColor, visibility);
}
