#include <array>

#include "Constants.hpp"
#include "Utils.h"
#pragma once

class Quad {
 private:
  glm::vec2 m_pos;
  float m_size;
  bool m_visible;

 public:
  static constexpr std::array<unsigned int, 6> faceindices = {0, 1, 2, 2, 3, 0};  // Face
  Quad(const glm::vec2& pos, float size);
  bool is_visible();
  void toggle_visible();
  std::vector<float> GenerateVertices();
};
