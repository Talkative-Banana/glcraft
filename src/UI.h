#pragma once
#include <memory>
#include <unordered_map>

#include "Constants.hpp"
#include "IndexBuffer.h"
#include "UIComponent.h"
#include "Utils.h"
#include "VertexArray.h"

class UI {
 private:
  uint32_t m_handle{}, cnt{};
  std::unordered_map<uint32_t, UIComponent> m_components;
  std::vector<float> m_quad_vertices;
  std::vector<GLuint> m_quad_indices;
  std::unique_ptr<VertexArray> m_quadva;

 public:
  uint32_t add_component(const UIComponent &);
  bool remove_component(const uint32_t);
  UIComponent &get_component(const uint32_t);

  void Render();
  void Draw();
  UI();
};
