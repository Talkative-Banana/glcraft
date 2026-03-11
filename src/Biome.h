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
  int m_type;
  GLboolean m_displayBiome;

  void allocate_chunks(bool);
  void setup_chunks(bool);

public:
  uint64_t m_id;
  glm::ivec3 m_biomePos;
  std::atomic_bool m_running{true};
  std::thread m_worker1, m_worker2;
  GLboolean m_dirtyBit{false};
  std::atomic<int> m_Run{0};
  std::atomic<int> m_chunksSetup{0}, m_chunksRerendered{0}, m_chunksFinished{0};
  std::atomic<BIOMESTATUS> m_RenderIter{BIOMESTATUS::IDLE};
  std::array<std::array<std::shared_ptr<Chunk>, CHUNK_COUNTZ>, CHUNK_COUNTX>
      m_chunks;
  std::unordered_map<uint64_t, std::weak_ptr<Chunk>> m_renderQueue;
  Biome(int, glm::ivec3, GLboolean);
  ~Biome();
  void SetupBiome(bool);
  void Draw(OBJ_TYPE, glm::dvec3);
  void Update_queue(glm::dvec3, glm::dmat4);
  void RecycleBiome(int, glm::ivec3, GLboolean);
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
