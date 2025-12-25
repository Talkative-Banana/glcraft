#pragma once
#include <memory>
#include <unordered_map>

#include "Constants.hpp"
#include "Effect.h"
#include "IndexBuffer.h"
#include "Texture.h"
#include "Utils.h"
#include "VertexArray.h"

class ParticleSystem {
 private:
  uint32_t m_handle{}, m_cnt{};
  std::unordered_map<uint32_t, std::unique_ptr<Effect>> m_effects;
  std::vector<GLuint> m_quad_indices;
  std::unique_ptr<VertexArray> m_quadva;
  std::unique_ptr<VertexBuffer> m_vbo;
  std::unique_ptr<IndexBuffer> m_ibo;
  Texture m_effecttex;

 public:
  uint32_t add_effect(std::unique_ptr<Effect>);
  bool remove_effect(const uint32_t);
  Effect *get_effect(const uint32_t);

  void Render();
  void Draw(float);
  ParticleSystem(const std::string &);
};
