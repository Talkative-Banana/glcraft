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
#include <filesystem>

#ifdef BUILD_SERVER
extern std::vector<std::string> world_operations;
#endif

struct BiomeArray {
  using bptr = std::shared_ptr<Biome>;
  std::unordered_map<uint64_t, bptr> BiomeMap;
  std::vector<bptr> toRemove;
  std::mutex biome_mutex;

  bptr get(uint64_t x, uint64_t y, uint64_t z) {
    std::lock_guard<std::mutex> lock(biome_mutex);
    auto it = BiomeMap.find(index(x, y, z));
    return it == BiomeMap.end() ? nullptr : it->second;
  }

  void set(uint64_t i, uint64_t j, uint64_t k, bptr value) {
    std::lock_guard<std::mutex> lock(biome_mutex);
    uint64_t idx = index(i, j, k);
    if (value == nullptr) {
      auto it = BiomeMap.find(idx);
      if (it != BiomeMap.end()) {
        toRemove.push_back(it->second);
        BiomeMap.erase(it);
        return;
      }
    }
    BiomeMap[idx] = std::move(value);
  }

  uint64_t cleartoRemove() {
    bptr temp;
    {
      std::lock_guard<std::mutex> lock(biome_mutex);
      if (!toRemove.empty()) {
        temp = toRemove.back();
        toRemove.pop_back();
      }
    }
    // if dirty dump and store to disk
    if (temp && temp->dirtybit) {
      temp->save(std::to_string(temp->m_id));
    }
    // destruction happens here, outside lock
    return temp ? temp->m_id : 0;
  }

  bool isPresent(uint64_t idx) {
    std::lock_guard<std::mutex> lock(biome_mutex);
    return BiomeMap.find(idx) != BiomeMap.end();
  }

  uint64_t index(uint64_t x, uint64_t y, uint64_t z) const {
    return BIOME_COUNTX * BIOME_COUNTZ * y + BIOME_COUNTX * x + z;
  }
};

class World {
  friend class BiomeArray;

private:
  int m_seed;
  glm::ivec3 m_worldpos;
  WEATHER m_weather{WEATHER::CLOUDY};
  std::unordered_map<uint64_t, std::weak_ptr<Biome>> render_queue;
  std::queue<std::weak_ptr<Biome>> setup_queue;
  std::queue<std::weak_ptr<Biome>> rerender_queue;
  std::set<uint64_t> job_scheduled;
  void workerLoop();
  std::thread worker;
  std::atomic<bool> m_running{true};
  std::atomic<bool> run_rerender_task{false};

  std::mutex setup_mutex;
  std::condition_variable setup_cv;

  std::queue<std::tuple<int, int, int, glm::ivec3, bool>> job_queue;

public:
  BiomeArray biomes;
  std::unordered_map<uint, Chunk> load_map;
  std::unordered_map<uint, std::weak_ptr<Chunk>> save_map;
  std::queue<std::weak_ptr<Biome>> bind_queue;
  World(int, const glm::ivec3 &);
  ~World();
  void EnqueueVisibleBiomes(glm::dvec3);
  bool isSolid(const glm::ivec3 &);
  bool isStandable(const glm::ivec3 &);
  bool isVisible(const glm::ivec3 &);
  Block *get_block_by_center(const glm::ivec3 &);
  std::weak_ptr<Chunk> get_chunk_by_center(const glm::ivec3 &);
  std::weak_ptr<Biome> get_biome_by_center(const glm::ivec3 &);
  void save_model(std::shared_ptr<Chunk>, std::string);
  void load_model(glm::ivec3, std::string, bool refresh_chunk = true);
  void SetupBiomesPass1();
  void SetupBiomesPass2();
  void Draw(OBJ_TYPE, glm::dvec3);
  void Update_queue(glm::dvec3, glm::dmat4);
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
