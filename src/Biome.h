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
  std::thread worker1, worker2;
  std::atomic<int> chunks_ready{0};
  GLboolean dirtybit, isrerenderiter;
  std::array<std::array<std::shared_ptr<Chunk>, CHUNK_COUNTZ>, CHUNK_COUNTX>
      chunks;
  std::unordered_set<std::shared_ptr<Chunk>> render_queue;
  Biome(int t, glm::ivec3 pos, GLboolean display);
  void SetupBiome(bool firstRun);
  void Draw(OBJ_TYPE type, glm::vec3 cameraPos);
  void Update_queue(glm::vec3 playerpos, glm::mat4 VP);
};

struct Plane {
  glm::vec3 normal;
  float d;

  void normalize() {
    float len = glm::length(normal);
    normal /= len;
    d /= len;
  }

  float distance(const glm::vec3 &p) const { return glm::dot(normal, p) + d; }
};
