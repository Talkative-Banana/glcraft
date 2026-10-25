#include "Block.h"

Block::Block(const glm::ivec3 &pos, bool solid, BLOCK_TYPE bltype) {
  blmask = (static_cast<uint64_t>(bltype) << 23) | (pos.x << 10) |
           (pos.y << 5) | (pos.z);
  if (solid)
    blmask |= (1 << 15);
}

Block::Block() : blmask(0) {}

BLOCK_TYPE Block::get_type() const {
  return static_cast<BLOCK_TYPE>((blmask & TYPE_MASK) >> 23);
}

glm::ivec3 Block::get_pos(GLuint _blmask) {
  GLuint posx = (_blmask & BLOCK_POSX) >> 10;
  GLuint posy = (_blmask & BLOCK_POSY) >> 5;
  GLuint posz = (_blmask & BLOCK_POSZ);
  return glm::ivec3(posx, posy, posz);
}

glm::ivec3 Block::get_pos() const {
  GLuint posx = blmask & BLOCK_POSX;
  GLuint posy = blmask & BLOCK_POSY;
  GLuint posz = blmask & BLOCK_POSZ;
  return glm::ivec3(posx, posy, posz);
}

bool Block::is_transparent() const {
  bool transparent = (get_type() == BLOCK_TYPE::WATER_BLOCK) ||
                     (get_type() == BLOCK_TYPE::GLASS_BLOCK);
  return transparent;
}

bool Block::is_ref() { return (get_type() == BLOCK_TYPE::REF_BLOCK); }

bool Block::is_removable() {
  return (get_type() != BLOCK_TYPE::BEDROCK_BLOCK &&
          get_type() != BLOCK_TYPE::WATER_BLOCK);
}

bool Block::Render(uint32_t mask, uint32_t ambient_occ) {
  blmask &= 0xFFFFFFFFull & ~(1ull << 16); // clear old AO and the visible bit

  if (!is_solid() || mask == 0)
    return false;

  blmask |= 1ull << 16; // visible
  blmask |= static_cast<uint64_t>(ambient_occ) << 32;
  return true;
}

void Block::remove() {
  if (!is_removable())
    return;             // do not clear indestructible blocks
  blmask &= ~(3 << 15); // clear solid  and visible bit
}

void Block::add(BLOCK_TYPE bltype) {
  blmask |= (3 << 15); // add solid and visble bit
  blmask &= ~static_cast<uint64_t>(TYPE_MASK);
  blmask |= (static_cast<uint64_t>(bltype) << 23);
}
