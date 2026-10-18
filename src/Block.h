#include <array>

#include "Constants.hpp"
#include "Utils.h"
#pragma once

class Block {
public:
  static glm::ivec3 get_pos(uint32_t);
  static constexpr std::array<std::array<unsigned int, 6>, 6> faceindices = {
      {{{2, 3, 0, 2, 0, 1}},        // Back
       {{7, 6, 5, 7, 5, 4}},        // Front
       {{11, 10, 9, 11, 9, 8}},     // Left
       {{15, 14, 13, 15, 13, 12}},  // Right
       {{19, 18, 17, 19, 17, 16}},  // Top
       {{23, 22, 21, 23, 21, 20}}}, // Bottom
  };

  Block(const glm::ivec3 &pos, bool solid, BLOCK_TYPE bltype);
  Block();

  bool Render(uint32_t mask, uint32_t ambient_occ);
  void add(BLOCK_TYPE bltype);
  void remove();
  bool is_ref();
  bool is_removable();
  bool is_transparent();
  BLOCK_TYPE get_type();
  glm::ivec3 get_pos();

  inline bool is_solid() { return blmask & (1 << 15); }
  inline bool is_standable() {
    auto mask = blmask;
    return (mask & (1 << 15)) &&
           ((mask >> 23) != static_cast<uint32_t>(BLOCK_TYPE::WATER_BLOCK));
  }

  // visible solid
  // 00000000_00000000_00000000_00000000_00000000_00000000_00000000_00000000
  // ^^^^^^^^ ^^^^^^^^ ^^^^^^^^ ^^^^^^^^ ^^^^^^^^ ^^^^^^^^ ^^^^^^^^ ^^^^^^^^
  // |||||||| |||||||| |||||||| |||||||| |||||||| |||||||| |||||||| ||||||||
  // ######## ######## ######## oooooooo tttttttt tmmmmmmv sxxxxxyy yyyzzzzz
  //                                               btrlfb
  uint64_t blmask;
};
