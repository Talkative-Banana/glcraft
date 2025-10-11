#pragma once
#include <atomic>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_set>

#include "Chunk.h"
#include "Constants.hpp"
#include "IndexBuffer.h"
#include "VertexArray.h"

class Biome {
 private:
  int type;
  GLboolean displaybiome;

 public:
  GLuint x_cord, z_cord;
  glm::ivec3 Biomepos;
  std::atomic<int> chunks_ready{0};
  GLboolean dirtybit, isrerenderiter;
  std::array<std::array<std::shared_ptr<Chunk>, CHUNK_COUNTZ>, CHUNK_COUNTX> chunks;
  std::unordered_set<std::shared_ptr<Chunk>> render_queue;
  Biome(int t, glm::ivec3 pos, GLboolean display);
  void RenderBiome(bool firstRun);
  void Draw();
  void Update_queue(glm::vec3 playerpos, glm::vec3 playerForward, float fov);
};
