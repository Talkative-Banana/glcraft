#include "UI.h"

#include "Renderer.h"

UI::UI() {
  m_quadva = std::make_unique<VertexArray>();
}

bool UI::remove_component(const uint32_t handle) {
  if (m_components.find(handle) == m_components.end()) return false;

  m_components.erase(handle);
  return true;
}

uint32_t UI::add_component(const UIComponent& uicomp) {
  m_components.emplace(m_handle++, uicomp);
  return m_handle - 1;
}

UIComponent& UI::get_component(const uint32_t handle) {
  auto it = m_components.find(handle);
  if (it == m_components.end()) {
    throw std::out_of_range("UIComponent handle not found");
  }
  return it->second;
}

void UI::Render() {
  m_quad_vertices.clear();
  m_quad_indices.clear();
  m_cnt = 0;

  for (auto& [handle, component] : m_components) {
    Quad& quad = component.get_quad();
    auto vec = quad.GenerateVertices();
    for (int i = 0; i < vec.size(); i++) {
      m_quad_vertices.push_back(vec[i]);
    }
    for (auto index : Quad::faceindices) m_quad_indices.push_back(m_cnt + index);
    m_cnt += 4;
  }
  m_quadva->Bind();
  VertexBufferLayout layout;
  layout.Push(GL_FLOAT, 2);
  layout.Push(GL_FLOAT, 2);
  layout.Push(GL_FLOAT, 1);
  m_vbo = std::make_unique<VertexBuffer>(
      m_quad_vertices.data(), m_quad_vertices.size() * sizeof(float));
  m_quadva->AddBuffer(*m_vbo, layout);
  m_ibo = std::make_unique<IndexBuffer>(m_quad_indices.data(), m_quad_indices.size());
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void UI::Draw() {
  if (m_components.empty()) return;
  m_quadva->Bind();
  m_vbo->Bind();
  m_ibo->Bind();
  for (auto& [_, component] : m_components) {
    auto& tex = component.get_texture();
    tex.Bind();
    glDrawElements(GL_TRIANGLES, 6 * (m_cnt / 4), GL_UNSIGNED_INT, nullptr);
  }
}
