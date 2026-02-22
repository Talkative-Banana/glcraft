#include "ParticleSystem.h"

#include "Renderer.h"

ParticleSystem::ParticleSystem(const std::string &filepath)
    : m_effecttex(filepath) {
  m_quadva = std::make_unique<VertexArray>();
}

bool ParticleSystem::remove_effect(const uint32_t handle) {
  if (m_effects.find(handle) == m_effects.end())
    return false;

  m_effects.erase(handle);
  return true;
}

[[nodiscard]] uint32_t
ParticleSystem::add_effect(std::unique_ptr<Effect> effect) {
  m_effects.emplace(m_handle++, std::move(effect));
  return m_handle - 1;
}

Effect *ParticleSystem::get_effect(const uint32_t handle) {
  auto it = m_effects.find(handle);
  if (it == m_effects.end()) {
    throw std::out_of_range("Effect handle not found");
  }
  return it->second.get();
}

void ParticleSystem::Render() {
  // Clear existing state
  m_cnt = 0;
  m_quad_indices.clear();
  if (m_effects.empty())
    return;

  // Setup dynamic buffer
  m_quadva->Bind();
  VertexBufferLayout layout;
  layout.Push(GL_FLOAT, 2); // pos
  layout.Push(GL_FLOAT, 2); // uv cords
  // layout.Push(GL_FLOAT, 2);  // index
  layout.Push(GL_FLOAT, 1); // alpha
  uint32_t vecsize = 0;
  for (auto &[handle, effect] : m_effects)
    vecsize += effect->get_size();
  m_vbo = std::make_unique<VertexBuffer>(vecsize * sizeof(float) * 4 * 5);
  m_quadva->AddBuffer(*m_vbo, layout);

  for (auto &[handle, effect] : m_effects) {
    for (auto &particle : effect->get_particles()) {
      for (auto &idx : Quad::faceindices) {
        m_quad_indices.push_back(m_cnt + idx);
      }
      m_cnt += 4;
    }
  }
  m_ibo = std::make_unique<IndexBuffer>(m_quad_indices.data(),
                                        m_quad_indices.size());
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void ParticleSystem::Draw(float dt) {
  if (m_effects.empty())
    return;
  m_quadva->Bind();
  m_vbo->Bind();
  m_ibo->Bind();
  m_effecttex.Bind();
  uint32_t offset = 0;
  for (auto &[handle, effect] : m_effects) {
    effect->run(dt);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * offset * 4 * 5,
                    effect->get_data().size() * sizeof(float),
                    effect->get_data().data());
    offset += effect->get_size();
  }
  glDrawElements(GL_TRIANGLES, 6 * (m_cnt / 4), GL_UNSIGNED_INT, nullptr);
}
