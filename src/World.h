#pragma once

#include <array>
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <set>
#include <unordered_map>

#include "Biome.h"
#include "Constants.hpp"
#include "IndexBuffer.h"
#include "Renderer.h"
#include "Utils.h"
#include "VertexArray.h"

class World {
 private:
  int m_seed;
  glm::ivec3 m_worldpos;
  std::unordered_set<std::shared_ptr<Biome>> render_queue;
  std::queue<std::shared_ptr<Biome>> setup_queue;
  std::queue<std::shared_ptr<Biome>> rerender_queue;
  std::set<GLuint64> job_scheduled;
  void workerLoop();
  std::thread worker;
  std::atomic<bool> running{true};
  std::atomic<bool> run_rerender_task{false};

  std::mutex setup_mutex;
  std::condition_variable setup_cv;

  std::queue<std::tuple<int, int, glm::ivec3>> job_queue;

 public:
  std::array<std::array<std::shared_ptr<Biome>, BIOME_COUNTZ>, BIOME_COUNTX> biomes;
  std::unordered_map<uint, Chunk> load_map;
  std::unordered_map<uint, std::shared_ptr<Chunk>> save_map;
  World(int seed, const glm::ivec3 &pos);
  void SetupWorld(glm::vec3 playerpos);
  bool isSolid(const glm::ivec3 &pos);
  bool isVisible(const glm::ivec3 &pos);
  Block *get_block_by_center(const glm::ivec3 &pos);
  std::shared_ptr<Chunk> get_chunk_by_center(const glm::ivec3 &pos);
  std::shared_ptr<Biome> get_biome_by_center(const glm::ivec3 &pos);
  void save_model(std::shared_ptr<Chunk> chunk, std::string name);
  void load_model(glm::ivec3 pos, std::string model);
  void RenderWorld(bool firstRun);
  void Draw();
  void Update_queue(glm::vec3 playerpos, glm::vec3 playerForward, float fov);
  void save(std::string save_file);
  int getSeed();
  void DoBindTask(bool firstRun);
};
