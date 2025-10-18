#pragma once
#include <memory>
#include <thread>
#include <unordered_set>

#include "Chunk.h"
#include "Constants.hpp"
#include "IndexBuffer.h"
#include "VertexArray.h"

class Biome {
 private:
  int type;
  glm::ivec3 Biomepos;
  GLboolean displaybiome;
  std::unordered_set<std::shared_ptr<Chunk>> render_queue;

 public:
  GLboolean dirtybit;
  GLuint x_cord, z_cord;
  std::array<std::array<std::shared_ptr<Chunk>, CHUNK_COUNTZ>, CHUNK_COUNTX> chunks;
  Biome(int t, glm::ivec3 pos, GLboolean display);
  void RenderBiome(bool firstRun);
  void Draw();
  void Update_queue(glm::vec3 playerpos, glm::vec3 playerForward, float fov);
};
