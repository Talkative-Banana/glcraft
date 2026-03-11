#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <set>
#include <shared_mutex>
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
  std::vector<bptr> toRecycle;
  std::shared_mutex biome_mutex;

  bptr get(uint64_t x, uint64_t y, uint64_t z) {
    std::shared_lock<std::shared_mutex> lock(biome_mutex);
    auto it = BiomeMap.find(index(x, y, z));
    return it == BiomeMap.end() ? nullptr : it->second;
  }

  bptr set(uint64_t i, uint64_t j, uint64_t k, bool isNew, int t = 0,
           glm::ivec3 pos = {}, GLboolean display = 0) {
    uint64_t idx = index(i, j, k);
    if (!isNew) {
      std::unique_lock lock(biome_mutex);
      auto it = BiomeMap.find(idx);
      if (it != BiomeMap.end()) {
        toRemove.push_back(it->second);
        BiomeMap.erase(it);
      }
      return nullptr;
    }

    bptr temp;

    {
      std::unique_lock lock(biome_mutex);
      auto it = BiomeMap.find(idx);
      if (it != BiomeMap.end())
        return it->second;

      if (!toRecycle.empty()) {
        temp = toRecycle.back();
        toRecycle.pop_back();
        BiomeMap.emplace(idx, temp);
      }
    }

    if (temp) {
      temp->RecycleBiome(t, pos, display);
      return temp;
    }

    // expensive part outside lock
    temp = std::make_shared<Biome>(t, pos, display);

    {
      std::unique_lock lock(biome_mutex);

      auto [it, inserted] = BiomeMap.emplace(idx, temp);

      if (!inserted) {
        // another thread beat us
        return it->second;
      }
    }

    return temp;
  }

  uint64_t cleartoRemove() {
    bptr temp;
    {
      std::unique_lock<std::shared_mutex> lock(biome_mutex);
      if (!toRemove.empty()) {
        temp = toRemove.back();
        toRemove.pop_back();
        if (temp->m_Run) {
          toRecycle.push_back(temp);
        }
      }
    }
    // if dirty dump and store to disk
    if (temp && temp->m_dirtyBit) {
      temp->save(std::to_string(temp->m_id));
    }
    // destruction happens here, outside lock
    return temp ? temp->m_id : 0;
  }

  bool isPresent(uint64_t idx) {
    std::shared_lock<std::shared_mutex> lock(biome_mutex);
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
  std::unordered_map<uint64_t, std::weak_ptr<Biome>> m_renderQueue;
  std::queue<std::weak_ptr<Biome>> m_setupQueue;
  std::queue<std::weak_ptr<Biome>> m_rerenderQueue;
  std::queue<std::weak_ptr<Biome>> m_waitingQueue;
  std::set<uint64_t> m_jobScheduled;
  void workerLoop();
  std::thread m_worker;
  std::atomic<bool> m_running{true};
  std::atomic<bool> m_runRerenderTask{false};

  std::mutex m_setupMutex;
  std::condition_variable m_setupCv;

  std::queue<std::tuple<int, int, int, glm::ivec3, bool>> m_jobQueue;

public:
  BiomeArray m_biomes;
  std::unordered_map<uint, Chunk> m_loadMap;
  std::unordered_map<uint, std::weak_ptr<Chunk>> m_saveMap;
  std::queue<std::weak_ptr<Biome>> m_bindQueue;
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
  void load_model(glm::ivec3, std::string, bool = true);
  void SetupBiomesPass1();
  void SetupBiomesPass2();
  void Draw(OBJ_TYPE, glm::dvec3);
  void Update_queue(glm::dvec3, glm::dmat4);
  void save(std::string);
  int getSeed();
  WEATHER getWeather();
  void setWeather(WEATHER);
  void MarkBiomesReadyForPass1();
  void MarkBiomesReadyForPass2();
  void MarkBiomesReadyForBoundaryRemoval();

  void RefreshChunks(glm::ivec3, bool = false, bool = false, bool = false,
                     bool = false);

  void handleNetworkRequest(WorldState &);
  std::shared_ptr<std::string> handle_client_input(const std::string &);
  bool Valid(WorldState &);
  std::string get_state(WorldState &);
};
