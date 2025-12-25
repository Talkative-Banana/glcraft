#include "Effect.h"

Effect::Effect(uint32_t number) : m_size(number) {
  m_particles.clear();
  m_particles.reserve(number);
}

std::vector<Quad> Effect::get_particles() {
  return m_particles;
}

std::vector<float> Effect::get_data() {
  data.clear();
  data.reserve(m_particles.size() * 20);

  for (auto& p : m_particles) {
    auto verts = p.GenerateVertices();
    data.insert(data.end(), verts.begin(), verts.end());
  }
  return data;
}

uint32_t Effect::get_size() {
  return m_particles.size();
}
