#include "Quad.h"

#include "Renderer.h"

Quad::Quad(const glm::vec2& pos, float size) : m_pos(pos), m_size(size), m_visible(true) {
}

Quad::Quad(const glm::vec2& pos, const glm::vec2& vel, float size, bool ccw)
    : m_pos(pos), m_vel(vel), m_size(size), m_visible(true), m_ccw(ccw) {
}

bool Quad::is_visible() {
  return m_visible;
}

void Quad::toggle_visible() {
  m_visible ^= 1;
}

std::vector<float> Quad::GenerateVertices() {
  std::vector<float> verts;
  verts.reserve(20);

  // local quad centered at origin
  glm::vec2 local[4] = {
      {-0.5f, -0.5f},
      {+0.5f, -0.5f},
      {+0.5f, +0.5f},
      {-0.5f, +0.5f},
  };

  glm::vec2 uv[4] = {{0, 1}, {1, 1}, {1, 0}, {0, 0}};

  float c = std::cos(m_angle);
  float s = std::sin(m_angle);

  for (int i = 0; i < 4; i++) {
    // rotate in local space
    glm::vec2 rotated = {c * local[i].x - s * local[i].y, s * local[i].x + c * local[i].y};

    // scale + translate to world space
    glm::vec2 world = m_pos + rotated * m_size;

    verts.push_back(world.x);
    verts.push_back(world.y);
    verts.push_back(uv[i].x);
    verts.push_back(uv[i].y);
    verts.push_back(m_alpha);
  }

  return verts;
}

void Quad::update_pos(glm::vec2 delta, float dt) {
  m_pos += delta * dt;
  m_pos = glm::vec2(
      glm::cos(m_angle) * m_pos.x - glm::sin(m_angle) * m_pos.y,
      glm::sin(m_angle) * m_pos.x + glm::cos(m_angle) * m_pos.y);
}

void Quad::update_size(float scale) {
  m_size += scale;
}

void Quad::update(float damping, float dt) {
  // exponential damping
  float decay = std::exp(-damping * dt);
  m_pos += m_vel * dt;
  m_vel *= decay;
}

void Quad::update_alpha(float delta) {
  m_alpha *= delta;
  if (m_alpha <= 0.01) toggle_visible();
}

void Quad::update_angle(float delta, float dt) {
  m_angle += m_ccw ? delta * dt : -delta * dt;
}
