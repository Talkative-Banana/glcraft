#include "Quad.h"

#include "Renderer.h"

Quad::Quad(const glm::vec2& pos, float size) : m_pos(pos), m_size(size), m_visible(true) {
}

bool Quad::is_visible() {
  return m_visible;
}

void Quad::toggle_visible() {
  m_visible ^= 1;
}

std::vector<float> Quad::GenerateVertices() {
  std::vector<float> verts;

  verts.push_back(m_pos.x);
  verts.push_back(m_pos.y);
  verts.push_back(0.0f);
  verts.push_back(1.0f);

  verts.push_back(m_pos.x + m_size);
  verts.push_back(m_pos.y);
  verts.push_back(1.0f);
  verts.push_back(1.0f);

  verts.push_back(m_pos.x + m_size);
  verts.push_back(m_pos.y + m_size);
  verts.push_back(1.0f);
  verts.push_back(0.0f);

  verts.push_back(m_pos.x);
  verts.push_back(m_pos.y + m_size);
  verts.push_back(0.0f);
  verts.push_back(0.0f);

  return verts;
}
