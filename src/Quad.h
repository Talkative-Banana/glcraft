#include <array>

#include "Constants.hpp"
#include "Utils.h"
#pragma once

class Quad {
 private:
  glm::vec2 m_pos{}, m_vel{};
  float m_size{}, m_angle{}, m_alpha{1.0f};
  bool m_visible{true}, m_ccw{};

 public:
  static constexpr std::array<unsigned int, 6> faceindices = {0, 1, 2, 2, 3, 0};  // Face
  Quad(const glm::vec2&, float);
  Quad(const glm::vec2&, const glm::vec2&, float, bool);
  bool is_visible();
  void toggle_visible();
  void update(float, float);
  void update_pos(glm::vec2, float);
  void update_size(float);
  void update_alpha(float);
  void update_angle(float, float);
  std::vector<float> GenerateVertices();
};
