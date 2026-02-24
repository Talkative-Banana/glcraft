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

  std::queue<std::tuple<int, int, glm::ivec3>> job_queue;

public:
  std::array<std::array<std::shared_ptr<Biome>, BIOME_COUNTZ>, BIOME_COUNTX>
      biomes;
  std::unordered_map<uint, Chunk> load_map;
  std::mutex biome_mutex;
  std::unordered_map<uint, std::shared_ptr<Chunk>> save_map;
  std::queue<std::shared_ptr<Biome>> bind_queue;
  World(int, const glm::ivec3 &);
  void SetupWorld(glm::vec3);
  bool isSolid(const glm::ivec3 &);
  bool isStandable(const glm::ivec3 &);
  bool isVisible(const glm::ivec3 &);
  Block *get_block_by_center(const glm::ivec3 &);
  std::shared_ptr<Chunk> get_chunk_by_center(const glm::ivec3 &);
  std::shared_ptr<Biome> get_biome_by_center(const glm::ivec3 &);
  void save_model(std::shared_ptr<Chunk>, std::string);
  void load_model(glm::ivec3, std::string, bool refresh_chunk = true);
  void RenderWorld(bool);
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
