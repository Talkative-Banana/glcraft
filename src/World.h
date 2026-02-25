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
#include "UI.h"
#include "Utils.h"
#include "VertexArray.h"

#ifdef BUILD_SERVER
extern std::vector<std::string> world_operations;
#endif

struct BiomeArray {
  using bptr = std::shared_ptr<Biome>;
  std::unordered_map<uint32_t, bptr> BiomeMap;
  std::mutex biome_mutex;

  bptr get(int y, int x, int z) {
    std::lock_guard<std::mutex> lock(biome_mutex);
    auto it = BiomeMap.find(index(y, x, z));
    return it == BiomeMap.end() ? nullptr : it->second;
  }

  void set(int y, int x, int z, bptr value) {
    std::lock_guard<std::mutex> lock(biome_mutex);
    BiomeMap[index(y, x, z)] = std::move(value);
  }

  bool isPresent(uint32_t idx) {
    std::lock_guard<std::mutex> lock(biome_mutex);
    return BiomeMap.find(idx) != BiomeMap.end();
  }

private:
  uint32_t index(uint32_t y, uint32_t x, uint32_t z) const {
    return BIOME_COUNTX * BIOME_COUNTZ * y + BIOME_COUNTX * x + z;
  }
};

class World {
private:
  int m_seed;
  glm::ivec3 m_worldpos;
  WEATHER m_weather{WEATHER::CLOUDY};
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

  std::queue<std::tuple<int, int, int, glm::ivec3>> job_queue;

public:
  BiomeArray biomes;
  std::unordered_map<uint, Chunk> load_map;
  std::mutex biome_mutex;
  std::unordered_map<uint, std::shared_ptr<Chunk>> save_map;
  std::queue<std::shared_ptr<Biome>> bind_queue;
  World(int, const glm::ivec3 &);
  void EnqueueVisibleBiomes(glm::vec3);
  bool isSolid(const glm::ivec3 &);
  bool isStandable(const glm::ivec3 &);
  bool isVisible(const glm::ivec3 &);
  Block *get_block_by_center(const glm::ivec3 &);
  std::shared_ptr<Chunk> get_chunk_by_center(const glm::ivec3 &);
  std::shared_ptr<Biome> get_biome_by_center(const glm::ivec3 &);
  void save_model(std::shared_ptr<Chunk>, std::string);
  void load_model(glm::ivec3, std::string, bool refresh_chunk = true);
  void SetupBiomesPass1();
  void SetupBiomesPass2();
  void Draw(OBJ_TYPE);
  void Update_queue(glm::vec3, glm::mat4);
  void save(std::string);
  int getSeed();
  WEATHER getWeather();
  void setWeather(WEATHER weather);
  void DoBindTask(bool);
  void RefreshChunks(glm::ivec3, bool left = false, bool back = false,
                     bool right = false, bool front = false);

  void handleNetworkRequest(WorldState &);
  std::shared_ptr<std::string> handle_client_input(const std::string &);
  bool Valid(WorldState &);
  std::string get_state(WorldState &);
};
