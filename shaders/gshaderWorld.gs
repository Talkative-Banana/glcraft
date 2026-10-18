#version 330 core
layout(points) in;
layout(triangle_strip, max_vertices = 36) out;   // 6 faces x 2 triangles x 3

flat in uvec2 gData[];

const uint REF_BLOCK_X  = (6u) | (6u << 4u) | (6u << 8u) | (6u << 12u) | (6u << 16u) | (6u << 20u);
const uint REF_BLOCK_Y  = (2u) | (2u << 4u) | (2u << 8u) | (2u << 12u) | (2u << 16u) | (2u << 20u);
const uint DIRT_BLOCK_X  = (2u) | (2u << 4u) | (2u << 8u) | (2u << 12u) | (2u << 16u) | (2u << 20u);
const uint DIRT_BLOCK_Y  = (0u) | (0u << 4u) | (0u << 8u) | (0u << 12u) | (0u << 16u) | (0u << 20u);
const uint STONE_BLOCK_X  = (1u) | (1u << 4u) | (1u << 8u) | (1u << 12u) | (1u << 16u) | (1u << 20u);
const uint STONE_BLOCK_Y  = (0u) | (0u << 4u) | (0u << 8u) | (0u << 12u) | (0u << 16u) | (0u << 20u);
const uint TREE_BLOCK_X  = (4u) | (4u << 4u) | (4u << 8u) | (4u << 12u) | (5u << 16u) | (5u << 20u);
const uint TREE_BLOCK_Y  = (1u) | (1u << 4u) | (1u << 8u) | (1u << 12u) | (1u << 16u) | (1u << 20u);
const uint LEAF_BLOCK_X  = (5u) | (5u << 4u) | (5u << 8u) | (5u << 12u) | (5u << 16u) | (5u << 20u);
const uint LEAF_BLOCK_Y  = (3u) | (3u << 4u) | (3u << 8u) | (3u << 12u) | (3u << 16u) | (3u << 20u);
const uint GRASS_BLOCK_X = (3u) | (3u << 4u) | (3u << 8u) | (3u << 12u) | (0u << 16u) | (2u << 20u);
const uint GRASS_BLOCK_Y = (0u) | (0u << 4u) | (0u << 8u) | (0u << 12u) | (0u << 16u) | (0u << 20u);
const uint IRON_BLOCK_X  = (6u) | (6u << 4u) | (6u << 8u) | (6u << 12u) | (6u << 16u) | (6u << 20u);
const uint IRON_BLOCK_Y  = (1u) | (1u << 4u) | (1u << 8u) | (1u << 12u) | (1u << 16u) | (1u << 20u);
const uint WOOD_BLOCK_X  = (4u) | (4u << 4u) | (4u << 8u) | (4u << 12u) | (4u << 16u) | (4u << 20u);
const uint WOOD_BLOCK_Y  = (0u) | (0u << 4u) | (0u << 8u) | (0u << 12u) | (0u << 16u) | (0u << 20u);
const uint WATER_BLOCK_X  = (14u) | (14u << 4u) | (14u << 8u) | (14u << 12u) | (14u << 16u) | (14u << 20u);
const uint WATER_BLOCK_Y  = (12u) | (12u << 4u) | (12u << 8u) | (12u << 12u) | (12u << 16u) | (12u << 20u);
const uint GRAVEL_BLOCK_X  = (3u) | (3u << 4u) | (3u << 8u) | (3u << 12u) | (3u << 16u) | (3u << 20u);
const uint GRAVEL_BLOCK_Y  = (1u) | (1u << 4u) | (1u << 8u) | (1u << 12u) | (1u << 16u) | (1u << 20u);
const uint SAND_BLOCK_X  = (2u) | (2u << 4u) | (2u << 8u) | (2u << 12u) | (2u << 16u) | (2u << 20u);
const uint SAND_BLOCK_Y  = (1u) | (1u << 4u) | (1u << 8u) | (1u << 12u) | (1u << 16u) | (1u << 20u);
const uint BEDROCK_BLOCK_X  = (1u) | (1u << 4u) | (1u << 8u) | (1u << 12u) | (1u << 16u) | (1u << 20u);
const uint BEDROCK_BLOCK_Y  = (1u) | (1u << 4u) | (1u << 8u) | (1u << 12u) | (1u << 16u) | (1u << 20u);
const uint BRICK_BLOCK_X  = (7u) | (7u << 4u) | (7u << 8u) | (7u << 12u) | (7u << 16u) | (7u << 20u);
const uint BRICK_BLOCK_Y  = (0u) | (0u << 4u) | (0u << 8u) | (0u << 12u) | (0u << 16u) | (0u << 20u);
const uint SNOW_BLOCK_X  = (2u) | (2u << 4u) | (2u << 8u) | (2u << 12u) | (2u << 16u) | (2u << 20u);
const uint SNOW_BLOCK_Y  = (4u) | (4u << 4u) | (4u << 8u) | (4u << 12u) | (4u << 16u) | (4u << 20u);
const uint HARD_SNOW_BLOCK_X  = (8u) | (8u << 4u) | (8u << 8u) | (8u << 12u) | (8u << 16u) | (8u << 20u);
const uint HARD_SNOW_BLOCK_Y  = (1u) | (1u << 4u) | (1u << 8u) | (1u << 12u) | (1u << 16u) | (1u << 20u);
const uint LAVA_BLOCK_X  = (15u) | (15u << 4u) | (15u << 8u) | (15u << 12u) | (15u << 16u) | (15u << 20u);
const uint LAVA_BLOCK_Y  = (15u) | (15u << 4u) | (15u << 8u) | (15u << 12u) | (15u << 16u) | (15u << 20u);
const uint GLASS_BLOCK_X  = (1u) | (1u << 4u) | (1u << 8u) | (1u << 12u) | (1u << 16u) | (1u << 20u);
const uint GLASS_BLOCK_Y  = (3u) | (3u << 4u) | (3u << 8u) | (3u << 12u) | (3u << 16u) | (3u << 20u);
// Y |  X -->
//   V

// Pack block arrays as const arrays (faster, not re-written per-vertex).
const uint blocks_X[17] = uint[17](REF_BLOCK_X, DIRT_BLOCK_X, STONE_BLOCK_X, TREE_BLOCK_X, LEAF_BLOCK_X, GRASS_BLOCK_X, IRON_BLOCK_X, WOOD_BLOCK_X, WATER_BLOCK_X, GRAVEL_BLOCK_X, SAND_BLOCK_X, BEDROCK_BLOCK_X, BRICK_BLOCK_X, SNOW_BLOCK_X, HARD_SNOW_BLOCK_X, LAVA_BLOCK_X, GLASS_BLOCK_X);
const uint blocks_Y[17] = uint[17](REF_BLOCK_Y, DIRT_BLOCK_Y, STONE_BLOCK_Y, TREE_BLOCK_Y, LEAF_BLOCK_Y, GRASS_BLOCK_Y, IRON_BLOCK_Y, WOOD_BLOCK_Y, WATER_BLOCK_Y, GRAVEL_BLOCK_Y, SAND_BLOCK_Y, BEDROCK_BLOCK_Y, BRICK_BLOCK_Y, SNOW_BLOCK_Y, HARD_SNOW_BLOCK_Y, LAVA_BLOCK_Y, GLASS_BLOCK_Y);


uniform float side;
uniform vec3 chunkpos;
uniform mat4 vModel;
uniform mat4 vView;
uniform mat4 vProjection;

out vec2 tile;
out float isoscale;
out vec3 NormalDir;
out vec2 TexCoord;
out float aoFactor;
out float visibility;

const float density = 0.007;
const float gradient = 1.5;

const ivec3 NORMALS[6] = ivec3[6](
  ivec3(0, 0, -1), ivec3(0, 0, 1), ivec3(-1, 0, 0),
  ivec3(1, 0, 0),  ivec3(0, 1, 0), ivec3(0, -1, 0)
);

// Your GenerateVerticies order
const ivec3 CORNERS[24] = ivec3[24](
  ivec3(0,0,0), ivec3(0,1,0), ivec3(1,1,0), ivec3(1,0,0),   // back
  ivec3(0,0,1), ivec3(0,1,1), ivec3(1,1,1), ivec3(1,0,1),                  // front
  ivec3(0,0,0), ivec3(0,1,0), ivec3(0,1,1), ivec3(0,0,1),                  // left
  ivec3(1,0,1), ivec3(1,1,1), ivec3(1,1,0), ivec3(1,0,0),                  // right
  ivec3(0,1,1), ivec3(0,1,0), ivec3(1,1,0), ivec3(1,1,1),                  // top
  ivec3(0,0,0), ivec3(0,0,1), ivec3(1,0,1), ivec3(1,0,0)                   // bottom
);

// Your faceindices table
const int FACE_IDX[36] = int[36](
  2,3,0, 2,0,1,         7,6,5, 7,5,4,
  11,10,9, 11,9,8,      15,14,13, 15,13,12,
  19,18,17, 19,17,16,   23,22,21, 23,21,20
);

void setup_textures(ivec3 n, uint blktype, uint centeroff) {
  uint frontx  = (blocks_X[blktype] >> 8u)  & 15u, fronty  = (blocks_Y[blktype] >> 8u)  & 15u;
  uint backx   = (blocks_X[blktype] >> 0u)  & 15u, backy   = (blocks_Y[blktype] >> 0u)  & 15u;
  uint topx    = (blocks_X[blktype] >> 16u) & 15u, topy    = (blocks_Y[blktype] >> 16u) & 15u;
  uint bottomx = (blocks_X[blktype] >> 20u) & 15u, bottomy = (blocks_Y[blktype] >> 20u) & 15u;
  uint leftx   = (blocks_X[blktype] >> 4u)  & 15u, lefty   = (blocks_Y[blktype] >> 4u)  & 15u;
  uint rightx  = (blocks_X[blktype] >> 12u) & 15u, righty  = (blocks_Y[blktype] >> 12u) & 15u;

  vec2 uv;
  if (n.z != 0) {
    uv = vec2((centeroff & 4u) >> 2, (centeroff & 2u) >> 1);
    if (n.z > 0) { tile = vec2(frontx, fronty); }
    else         { tile = vec2(backx, backy); uv.x = 1.0 - uv.x; }
  } else if (n.y != 0) {
    uv = vec2((centeroff & 4u) >> 2, (centeroff & 1u));
    if (n.y > 0) { tile = vec2(topx, topy); }
    else         { tile = vec2(bottomx, bottomy); uv.x = 1.0 - uv.x; }
  } else {
    uv = vec2((centeroff & 1u), (centeroff & 2u) >> 1);
    if (n.x < 0) { tile = vec2(leftx, lefty); }
    else         { tile = vec2(rightx, righty); uv.x = 1.0 - uv.x; }
  }
  TexCoord = uv;
}

// 00000000_00000000_00000000_00000000_00000000_00000000_00000000_00000000
// ^^^^^^^^ ^^^^^^^^ ^^^^^^^^ ^^^^^^^^ ^^^^^^^^ ^^^^^^^^ ^^^^^^^^ ^^^^^^^^
// |||||||| |||||||| |||||||| |||||||| |||||||| |||||||| |||||||| ||||||||
// ######## ######## ######## oooooooo tttttttt tmmmmmmv sxxxxxyy yyyzzzzz
//                                               btrlfb


void main() {
  uint u_vVertex = gData[0].x;
  uint l_vVertex = gData[0].y;
  vec3 base   = vec3(float((l_vVertex >> 10u) & 31u), float((l_vVertex >> 5u) & 31u), float(l_vVertex & 31u));

  uint mask   = (l_vVertex >> 17u) & 63u;
  uint blktype = min((l_vVertex >> 23u) & 511u, 16u);
  uint ac = u_vVertex & 511u;

  for (int f = 0; f < 6; ++f) {
    if (((mask >> uint(f)) & 1u) == 0u) continue;

    ivec3 n = NORMALS[f];
    NormalDir = vec3(n);
    isoscale = (n.z != 0) ? 0.980 : (n.y != 0) ? 0.800 : 0.608;

    for (int t = 0; t < 6; ++t) {
      ivec3 c = CORNERS[FACE_IDX[f * 6 + t]];

      // Old `cent` field: corner id with every bit inverted
      uint centeroff = uint((1 - c.x) * 4 + (1 - c.y) * 2 + (1 - c.z));
      setup_textures(n, blktype, centeroff);

      // Ambient occlusion: top face only, keyed to the corner
      float ao = 1.0;
      if (f == 4) {
        uint w;
        if (c.x == 0) w = (c.z == 0) ? (ac & 7u) : ((ac >> 2u) & 7u);
        else          w = (c.z == 1) ? ((ac >> 4u) & 7u) : ((ac >> 6u) & 7u);
        uint s1 = w & 1u, dg = (w >> 1u) & 1u, s2 = (w >> 2u) & 1u;
        uint occ = (s1 != 0u && s2 != 0u) ? 3u : (s1 + s2 + dg);
        ao = 1.0 - float(occ) * 0.25;
      }
      aoFactor = ao;

      vec3 pos = side * (base + vec3(c));
      vec4 world = vec4(pos + chunkpos, 1.0);
      gl_Position = vProjection * vView * vModel * world;

      float dist = length((vView * world).xyz);
      visibility = clamp(exp(-pow(dist * density, gradient)), 0.0, 1.0);

      EmitVertex();
      if (t == 2 || t == 5) EndPrimitive();   // keep triangles separate
    }
  }
}
