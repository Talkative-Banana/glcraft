#include "UIComponent.h"

#include "Renderer.h"

UIComponent::UIComponent(const std::string& filepath, const glm::vec2& pos, float size)
    : m_texture(filepath), m_quad(pos, size) {
}

bool UIComponent::is_visible() {
  return m_quad.is_visible();
}

void UIComponent::toggle_visible() {
  m_quad.toggle_visible();
}

Texture& UIComponent::get_texture() {
  return m_texture;
}

Quad& UIComponent::get_quad() {
  return m_quad;
}
