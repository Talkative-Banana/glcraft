#include "Block.h"

#include "Renderer.h"

Block::Block(const glm::ivec3 &pos, GLboolean solid, BLOCK_TYPE bltype) {
  blmask = (static_cast<uint32_t>(bltype) << 23) | (pos.x << 10) |
           (pos.y << 5) | (pos.z);
  if (solid)
    blmask |= (1 << 15);
}

Block::Block() : blmask(0) {}

GLuint Block::Mask(GLuint X, GLuint Y, GLuint Z, GLuint cent, GLuint normal,
                   GLuint bltype, GLuint ac) {
  GLuint mask = 0;
  // oootttttnnnccczzzzzzyyyyyyxxxxxx 7 all add [block can have their final
  // point at 32]
  //            xyz
  mask |= X | (Y << 6) | (Z << 12) | (cent << 18) | (normal << 21) |
          (bltype << 24) | (ac << 29);
  return mask;
}

BLOCK_TYPE Block::get_type() {
  return static_cast<BLOCK_TYPE>((blmask & TYPE_MASK) >> 23);
}

glm::ivec3 Block::get_pos(GLuint _blmask) {
  GLuint posx = (_blmask & BLOCK_POSX) >> 10;
  GLuint posy = (_blmask & BLOCK_POSY) >> 5;
  GLuint posz = (_blmask & BLOCK_POSZ);
  return glm::ivec3(posx, posy, posz);
}

glm::ivec3 Block::get_pos() {
  GLuint posx = blmask & BLOCK_POSX;
  GLuint posy = blmask & BLOCK_POSY;
  GLuint posz = blmask & BLOCK_POSZ;
  return glm::ivec3(posx, posy, posz);
}

void Block::GenerateVerticies(GLuint ambient_occ,
                              std::vector<GLuint> &vertices) {
  GLuint x = (blmask >> 10) & 31, y = (blmask >> 5) & 31, z = (blmask) & 31,
         blktype = (blmask >> 23) & 63;
  // Vertex Position
  // 24 verticies per block

  // Back Face 0123
  vertices.push_back(Mask(x, y, z, 7, 0, blktype, 0)); // 0
  // verticies.push_back({x - hside, y - hside, z - hside, x, y, z}); // 0 0
  vertices.push_back(Mask(x, y + 1, z, 5, 0, blktype, ambient_occ & 7)); // 1
  // verticies.push_back({x - hside, y + hside, z - hside, x, y, z}); // 1 1
  vertices.push_back(
      Mask(x + 1, y + 1, z, 1, 0, blktype, (ambient_occ >> 6) & 7)); // 2
  // verticies.push_back({x + hside, y + hside, z - hside, x, y, z}); // 2 2
  vertices.push_back(Mask(x + 1, y, z, 3, 0, blktype, 0)); // 3
  // verticies.push_back({x + hside, y - hside, z - hside, x, y, z}); // 3 3

  // Front Face 4567
  vertices.push_back(Mask(x, y, z + 1, 6, 1, blktype, 0)); // 4
  // verticies.push_back({x - hside, y - hside, z + hside, x, y, z}); // 4 4
  vertices.push_back(
      Mask(x, y + 1, z + 1, 4, 1, blktype, (ambient_occ >> 2) & 7)); // 5
  // verticies.push_back({x - hside, y + hside, z + hside, x, y, z}); // 5 5
  vertices.push_back(
      Mask(x + 1, y + 1, z + 1, 0, 1, blktype, (ambient_occ >> 4) & 7)); // 6
  // verticies.push_back({x + hside, y + hside, z + hside, x, y, z}); // 6 6
  vertices.push_back(Mask(x + 1, y, z + 1, 2, 1, blktype, 0)); // 7
  // verticies.push_back({x + hside, y - hside, z + hside, x, y, z}); // 7 7

  // Left Face 0154
  vertices.push_back(Mask(x, y, z, 7, 2, blktype, 0)); // 0
  // verticies.push_back({x - hside, y - hside, z - hside, x, y, z}); // 0 8
  vertices.push_back(Mask(x, y + 1, z, 5, 2, blktype, ambient_occ & 7)); // 1
  // verticies.push_back({x - hside, y + hside, z - hside, x, y, z}); // 1 9
  vertices.push_back(
      Mask(x, y + 1, z + 1, 4, 2, blktype, (ambient_occ >> 2) & 7)); // 5
  // verticies.push_back({x - hside, y + hside, z + hside, x, y, z}); // 5 10
  vertices.push_back(Mask(x, y, z + 1, 6, 2, blktype, 0)); // 4
  // verticies.push_back({x - hside, y - hside, z + hside, x, y, z}); // 4 11

  // Right Face 7623
  vertices.push_back(Mask(x + 1, y, z + 1, 2, 3, blktype, 0)); // 7
  // verticies.push_back({x + hside, y - hside, z + hside, x, y, z}); // 7 12
  vertices.push_back(
      Mask(x + 1, y + 1, z + 1, 0, 3, blktype, (ambient_occ >> 4) & 7)); // 6
  // verticies.push_back({x + hside, y + hside, z + hside, x, y, z}); // 6 13
  vertices.push_back(
      Mask(x + 1, y + 1, z, 1, 3, blktype, (ambient_occ >> 6) & 7)); // 2
  // verticies.push_back({x + hside, y + hside, z - hside, x, y, z}); // 2 14
  vertices.push_back(Mask(x + 1, y, z, 3, 3, blktype, 0)); // 3
  // verticies.push_back({x + hside, y - hside, z - hside, x, y, z}); // 3 15

  // Top Face 5126
  vertices.push_back(
      Mask(x, y + 1, z + 1, 4, 4, blktype, (ambient_occ >> 2) & 7)); // 5
  // verticies.push_back({x - hside, y + hside, z + hside, x, y, z}); // 5 16
  vertices.push_back(Mask(x, y + 1, z, 5, 4, blktype, ambient_occ & 7)); // 1
  // verticies.push_back({x - hside, y + hside, z - hside, x, y, z}); // 1 17
  vertices.push_back(
      Mask(x + 1, y + 1, z, 1, 4, blktype, (ambient_occ >> 6) & 7)); // 2
  // verticies.push_back({x + hside, y + hside, z - hside, x, y, z}); // 2 18
  vertices.push_back(
      Mask(x + 1, y + 1, z + 1, 0, 4, blktype, (ambient_occ >> 4) & 7)); // 6
  // verticies.push_back({x + hside, y + hside, z + hside, x, y, z}); // 6 19

  // Bottom Face 0473
  vertices.push_back(Mask(x, y, z, 7, 5, blktype, 0)); // 0
  // verticies.push_back({x - hside, y - hside, z - hside, x, y, z}); // 0 20
  vertices.push_back(Mask(x, y, z + 1, 6, 5, blktype, 0)); // 4
  // verticies.push_back({x - hside, y - hside, z + hside, x, y, z}); // 4 21
  vertices.push_back(Mask(x + 1, y, z + 1, 2, 5, blktype, 0)); // 7
  // verticies.push_back({x + hside, y - hside, z + hside, x, y, z}); // 7 22
  vertices.push_back(Mask(x + 1, y, z, 3, 5, blktype, 0)); // 3
  // verticies.push_back({x + hside, y - hside, z - hside, x, y, z}); // 3 23
}

bool Block::is_transparent() {
  bool transparent = (get_type() == BLOCK_TYPE::WATER_BLOCK) ||
                     (get_type() == BLOCK_TYPE::GLASS_BLOCK);
  return transparent;
}

bool Block::is_ref() { return (get_type() == BLOCK_TYPE::REF_BLOCK); }

bool Block::is_removable() {
  return (get_type() != BLOCK_TYPE::BEDROCK_BLOCK &&
          get_type() != BLOCK_TYPE::WATER_BLOCK);
}

GLuint Block::Render(GLuint mask, GLuint ambient_occ, GLuint offset,
                     std::vector<GLuint> &indices,
                     std::vector<GLuint> &rendervert) {
  if (!is_solid())
    return 0; // not solid
  GenerateVerticies(ambient_occ, rendervert);

  GLuint idx = 0, icnt = 0;
  // If a transparent block
  while (mask != 0) {
    blmask |= (1 << 16); // mark them visible if any side is visble
    if (mask & 1) {
      for (int i = 0; i < 6; i++) {
        icnt++;
        indices.push_back(offset + faceindices[idx][i]);
      }
    }
    mask >>= 1, idx++;
  }
  return icnt;
}

void Block::remove() {
  if (!is_removable())
    return;             // do not clear indestructible blocks
  blmask &= ~(3 << 15); // clear solid  and visible bit
}

void Block::add(BLOCK_TYPE bltype) {
  blmask |= (3 << 15); // add solid and visble bit
  blmask &= ~(TYPE_MASK);
  blmask |= (static_cast<int>(bltype) << 23);
}

bool Block::is_solid() { return blmask & (1 << 15); }
bool Block::is_standable() {
  return is_solid() && get_type() != BLOCK_TYPE::WATER_BLOCK;
}
