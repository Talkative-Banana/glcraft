#include <array>

#include "Constants.hpp"
#include "Quad.h"
#include "Texture.h"
#include "Utils.h"
#pragma once

class UIComponent {
private:
  Quad m_quad;
  Texture m_texture;

public:
  UIComponent(const std::string &filepath, const glm::vec2 &pos, float size);
  bool is_visible();
  void toggle_visible();
  Texture &get_texture();
  Quad &get_quad();

  UIComponent(const UIComponent &) = delete;
  UIComponent &operator=(const UIComponent &) = delete;

  UIComponent(UIComponent &&) = default;
  UIComponent &operator=(UIComponent &&) = default;
};
