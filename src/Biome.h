#pragma once
#include <assert.h>
#include <atomic>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>

#include "Chunk.h"
#include "Constants.hpp"
#include "IndexBuffer.h"
#include "VertexArray.h"

class Biome {
private:
  int type;
  GLboolean displaybiome;

  void allocate_chunks();
  void setup_chunks(bool firstRun);

public:
  uint64_t m_id;
  glm::ivec3 Biomepos;
  std::atomic_bool m_running{true};
  std::thread worker1, worker2;
  GLboolean dirtybit;
  std::atomic<int> m_chunksSetup{0}, m_chunksRerendered{0}, m_chunksFinished{0};
  std::atomic<BIOMESTATUS> m_RenderIter{BIOMESTATUS::IDLE};
  std::array<std::array<std::shared_ptr<Chunk>, CHUNK_COUNTZ>, CHUNK_COUNTX>
      chunks;
  std::unordered_map<uint64_t, std::weak_ptr<Chunk>> render_queue;
  Biome(int t, glm::ivec3 pos, GLboolean display);
  ~Biome();
  void SetupBiome(bool firstRun);
  void Draw(OBJ_TYPE type, glm::dvec3 cameraPos);
  void Update_queue(glm::dvec3 playerpos, glm::dmat4 VP);
  void save(std::string);
};

struct Plane {
  glm::dvec3 normal;
  float d;

  void normalize() {
    float len = glm::length(normal);
    normal /= len;
    d /= len;
  }

  float distance(const glm::dvec3 &p) const { return glm::dot(normal, p) + d; }
};
