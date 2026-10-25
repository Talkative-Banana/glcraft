#include <array>

#include "Constants.hpp"
#include "Utils.h"
#pragma once

class Block {
public:
  static glm::ivec3 get_pos(uint32_t);

  Block(const glm::ivec3 &pos, bool solid, BLOCK_TYPE bltype);
  Block();

  bool Render(uint32_t mask, uint32_t ambient_occ);
  void add(BLOCK_TYPE bltype);
  void remove();
  bool is_ref();
  bool is_removable();
  bool is_transparent() const;
  BLOCK_TYPE get_type() const;
  glm::ivec3 get_pos() const;

  inline bool is_solid() const { return blmask & (1 << 15); }
  inline bool is_standable() const {
    return is_solid() && get_type() != BLOCK_TYPE::WATER_BLOCK;
  }

  // visible solid
  // 00000000_00000000_00000000_00000000_00000000_00000000_00000000_00000000
  // ^^^^^^^^ ^^^^^^^^ ^^^^^^^^ ^^^^^^^^ ^^^^^^^^ ^^^^^^^^ ^^^^^^^^ ^^^^^^^^
  // |||||||| |||||||| |||||||| |||||||| |||||||| |||||||| |||||||| ||||||||
  // ######## ######## ######## oooooooo tttttttt tmmmmmmv sxxxxxyy yyyzzzzz
  //                                               btrlfb
  uint64_t blmask;
};
