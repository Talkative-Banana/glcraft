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
  return m_handle;
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
  cnt = 0;

  for (auto& [handle, component] : m_components) {
    Quad& quad = component.get_quad();
    auto vec = quad.GenerateVertices();
    for (auto points : vec) {
      m_quad_vertices.push_back(points);
    }
    for (auto index : Quad::faceindices) m_quad_indices.push_back(cnt + index);
    m_quadva->Bind();
    VertexBufferLayout layout;
    layout.Push(GL_FLOAT, 2);
    layout.Push(GL_FLOAT, 2);
    VertexBuffer vb(m_quad_vertices.data(), m_quad_vertices.size() * sizeof(GL_FLOAT));
    m_quadva->AddBuffer(vb, layout);
    IndexBuffer ib(m_quad_indices.data(), m_quad_indices.size());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    cnt += 4;
  }
}

void UI::Draw() {
  for (auto& [handle, component] : m_components) {
    m_quadva->Bind();
    auto& tex = component.get_texture();
    tex.Bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
  }
}
