#include "World.h"

#include <fstream>

std::mutex m;
std::queue<glm::ivec3> refreshq;

World::World(int seed, const glm::ivec3 &pos) : m_seed(seed), m_worldpos(pos) {
  // Read saved files if any
  std::ifstream input_bin_file("save/ff1.bin", std::ios::binary);
  if (input_bin_file) {
    int count = 1;
    input_bin_file.read(reinterpret_cast<char *>(&count), sizeof(count));
    for (int i = 0; i < count; i++) {
      Chunk chunk;
      if (!chunk.Deserialize(input_bin_file))
        break;
      std::cout << "Loaded a chunk with ID: " << chunk.m_saveId << '\n';
      m_loadMap.emplace(chunk.m_saveId, std::move(chunk));
    }
  } else {
    std::cerr << "Failed to open save file.\n";
  }

  Chunk::SetupNoise(m_seed);
  // start the worker after setting up all the members
  m_worker = std::thread(&World::workerLoop, this);
};

World::~World() {
  m_running = false;
  m_setupCv.notify_all();

  if (m_worker.joinable()) {
    m_worker.join();
  }
}

Block *World::get_block_by_center(const glm::ivec3 &pos) {
  // get the biome
  // get x and z cords
  glm::ivec3 pos_cpy = pos - glm::ivec3(HALF_BLOCK_SIZE);
  int x = pos_cpy.x / (BLOCK_SIZE), y = pos_cpy.y / (BLOCK_SIZE),
      z = pos_cpy.z / (BLOCK_SIZE);

  if (pos_cpy.x < 0 || pos_cpy.z < 0)
    return nullptr;

  // get the biome
  int biomex = x / (CHUNK_BLOCK_COUNT * CHUNK_COUNTX),
      biomez = z / (CHUNK_BLOCK_COUNT * CHUNK_COUNTZ),
      biomey = y / (CHUNK_BLOCK_COUNT);

  if (biomex >= BIOME_COUNTX || biomez >= BIOME_COUNTZ ||
      biomey >= BIOME_COUNTY)
    return nullptr;

  auto biome = m_biomes.get(biomex, biomey, biomez);
  if (!biome)
    return nullptr;
  // get the chunk

  // get the chunk
  int chunkx = (x / CHUNK_BLOCK_COUNT) % CHUNK_COUNTX,
      chunkz = (z / CHUNK_BLOCK_COUNT) % CHUNK_COUNTZ;

  auto chunk = biome->m_chunks[chunkx][chunkz];

  if (!chunk)
    return nullptr;
  auto &block = chunk->m_blocks[x % CHUNK_BLOCK_COUNT][y % CHUNK_BLOCK_COUNT]
                               [z % CHUNK_BLOCK_COUNT];
  return &block;
}

std::weak_ptr<Chunk> World::get_chunk_by_center(const glm::ivec3 &pos) {
  // get the biome
  // get x and z cords
  glm::ivec3 pos_cpy = pos - glm::ivec3(HALF_BLOCK_SIZE);
  int x = pos_cpy.x / (BLOCK_SIZE), z = pos_cpy.z / (BLOCK_SIZE),
      y = pos_cpy.y / (BLOCK_SIZE);

  if (pos_cpy.x < 0 || pos_cpy.z < 0)
    return std::weak_ptr<Chunk>{};

  // get the biome
  int biomex = x / (CHUNK_BLOCK_COUNT * CHUNK_COUNTX),
      biomez = z / (CHUNK_BLOCK_COUNT * CHUNK_COUNTZ),
      biomey = y / (CHUNK_BLOCK_COUNT);

  if (biomex >= BIOME_COUNTX || biomez >= BIOME_COUNTZ ||
      biomey >= BIOME_COUNTY)
    return std::weak_ptr<Chunk>{};

  auto biome = m_biomes.get(biomex, biomey, biomez);
  if (!biome)
    return std::weak_ptr<Chunk>{};

  // get the chunk
  int chunkx = (x / CHUNK_BLOCK_COUNT) % CHUNK_COUNTX,
      chunkz = (z / CHUNK_BLOCK_COUNT) % CHUNK_COUNTZ;

  auto chunk = biome->m_chunks[chunkx][chunkz];

  return chunk;
}

std::weak_ptr<Biome> World::get_biome_by_center(const glm::ivec3 &pos) {
  // get the biome
  // get x and z cords
  glm::ivec3 pos_cpy = pos - glm::ivec3(HALF_BLOCK_SIZE);
  int x = pos_cpy.x / (BLOCK_SIZE), z = pos_cpy.z / (BLOCK_SIZE),
      y = pos_cpy.y / (BLOCK_SIZE);

  if (pos_cpy.x < 0 || pos_cpy.z < 0)
    return std::weak_ptr<Biome>{};

  // get the biome
  int biomex = x / (CHUNK_BLOCK_COUNT * CHUNK_COUNTX),
      biomez = z / (CHUNK_BLOCK_COUNT * CHUNK_COUNTZ),
      biomey = y / (CHUNK_BLOCK_COUNT);

  if (biomex >= BIOME_COUNTX || biomez >= BIOME_COUNTZ ||
      biomey >= BIOME_COUNTY)
    return std::weak_ptr<Biome>{};

  auto biome = m_biomes.get(biomex, biomey, biomez);
  if (!biome)
    return std::weak_ptr<Biome>{};

  return biome;
}

bool World::isSolid(const glm::ivec3 &pos) {
  Block *b = get_block_by_center(pos);
  return (b && (((b->blmask) >> 15) & 1) == 1);
}

bool World::isStandable(const glm::ivec3 &pos) {
  Block *b = get_block_by_center(pos);
  return (b && b->is_solid() && b->get_type() != BLOCK_TYPE::WATER_BLOCK);
}

bool World::isVisible(const glm::ivec3 &pos) {
  Block *b = get_block_by_center(pos);
  return (b && (((b->blmask) >> 16) & 1) == 1);
}

void World::workerLoop() {
  while (m_running) {
    // Add and Remove Biome
    {
      std::unique_lock<std::mutex> lock(m_setupMutex);
      m_setupCv.wait(lock,
                     [this] { return !m_jobQueue.empty() || !m_running; });
      if (!m_running) {
        lock.unlock();
        break;
      }
      auto [i, j, k, pos, isremove] = m_jobQueue.front();
      m_jobQueue.pop();
      lock.unlock();

      if (isremove) {
        // Remove all of the unwanted biomes
        while (true) {
          uint64_t id = m_biomes.cleartoRemove();
          if (id) {
            std::lock_guard<std::mutex> lck(m_setupMutex);
            m_jobScheduled.erase(id);
          } else {
            break;
          }
        }
        continue; // Was a deletion request
      }
      std::string biomeId = std::to_string(m_biomes.index(i, k, j));
      std::string file_name = "save/tmp/" + biomeId + ".bin";
      // heavy work outside lock
      if (m_biomes.get(i, k, j)) {
        continue;
      } else if (std::filesystem::exists(file_name)) {
        std::ifstream input_bin_file(file_name, std::ios::binary);
        if (!input_bin_file) {
          std::cerr << "Failed to open biome file.\n";
          return;
        }
        int count = 1;
        input_bin_file.read(reinterpret_cast<char *>(&count), sizeof(count));
        for (int i = 0; i < count; i++) {
          Chunk chunk;
          if (!chunk.Deserialize(input_bin_file))
            break;
          std::cout << "Loading chunk: " << chunk.m_saveId << '\n';
          m_loadMap.emplace(chunk.m_saveId, std::move(chunk));
        }
      }
      auto biome = m_biomes.set(i, k, j, true, 0, pos, true);
      {
        std::lock_guard<std::mutex> g(m_setupMutex);
        m_setupQueue.push({biome->m_Run, biome});
      }
    }
  }
}

void World::EnqueueVisibleBiomes(glm::dvec3 playerpos) {
  // Do not set up for all the biomes
  if (playerpos.x < 0 || playerpos.y < 0 || playerpos.z < 0) {
    return;
  }
  uint64_t player_k = playerpos.y / BIOME_HEIGHT;
  uint64_t player_j = playerpos.z / BIOME_LENGTH;
  uint64_t player_i = playerpos.x / BIOME_LENGTH;

  uint64_t mink = player_k < 2 ? 0 : player_k - 2;
  uint64_t mini = player_i < 2 ? 0 : player_i - 2;
  uint64_t minj = player_j < 2 ? 0 : player_j - 2;

  uint64_t maxk = player_k >= BIOME_COUNTY - 2 ? BIOME_COUNTY - 1 : player_k;
  uint64_t maxi =
      player_i >= BIOME_COUNTX - 2 ? BIOME_COUNTX - 1 : player_i + 2;
  uint64_t maxj =
      player_j >= BIOME_COUNTZ - 2 ? BIOME_COUNTZ - 1 : player_j + 2;

  for (uint64_t k = maxk + 1; k-- > mink;) {
    for (uint64_t i = mini; i <= maxi; i++) {
      for (uint64_t j = minj; j <= maxj; j++) {
        uint64_t idx = k * (BIOME_COUNTX * BIOME_COUNTZ) + BIOME_COUNTX * i + j;

        glm::ivec3 biome_pos =
            glm::ivec3(BIOME_LENGTH * i, BIOME_HEIGHT * k, BIOME_LENGTH * j);
        bool insideX = abs(biome_pos.x + BIOME_LENGTH - playerpos.x) <=
                       RENDER_DISTANCE,
             insideZ = abs(biome_pos.z + BIOME_LENGTH - playerpos.z) <=
                       RENDER_DISTANCE,
             insideY = abs(biome_pos.y - playerpos.y) <= CHUNK_HEIGHT * 2;
        if (insideX && insideZ && insideY) {
          bool shouldSchedule = false;
          {
            std::lock_guard<std::mutex> lock(m_setupMutex);
            bool isPresent = m_jobScheduled.find(idx) != m_jobScheduled.end();
            // Costly move it to a seprate thread
            if (!isPresent) { // Addition of a new biome
              m_jobQueue.emplace(i, j, k, m_worldpos + biome_pos, false);
              m_jobScheduled.insert(idx);
              shouldSchedule = true;
            }
          }
          if (shouldSchedule) {
            m_setupCv.notify_one();
          }
        }
      }
    }
  }
}

void World::SetupBiomesPass1() {
  while (true) {
    std::weak_ptr<Biome> b_weak;
    uint32_t t_run = 0;
    {
      std::lock_guard<std::mutex> lock(m_setupMutex);
      if (m_setupQueue.empty())
        break;

      auto &[run, weak] = m_setupQueue.front();
      t_run = run, b_weak = weak;
      m_setupQueue.pop();
    }
    auto b = b_weak.lock();
    if (b && b->m_Run == t_run) {
      b->SetupBiome(true);
      m_bindQueue.push({t_run, b});
      m_renderQueue[b->m_id] = b_weak;
    }
  }
}

// Responsible for removing hidden block faces
void World::SetupBiomesPass2() {
  while (!m_rerenderQueue.empty()) {
    auto &[run, b_weak] = m_rerenderQueue.front();
    m_rerenderQueue.pop();

    auto b = b_weak.lock();
    if (b && b->m_Run == run) {
      b->SetupBiome(false); // ReRun
      m_bindQueue.push({run, b});
    }
  }
}

void World::Draw(OBJ_TYPE type, glm::dvec3 cameraPos) {
  // Do not render all the biomes just what world wants to using its
  // render_queue
  for (auto [_, b_weak] : m_renderQueue) {
    if (auto biome = b_weak.lock()) {
      constexpr auto count = CHUNK_COUNTX * CHUNK_COUNTZ;
      if (biome->m_chunksSetup.load(std::memory_order_acquire) == count) {
        biome->Draw(type, cameraPos);
      }
    }
  }
}

void World::Update_queue(glm::dvec3 playerpos, glm::dmat4 VP) {
  // Check for all the biomes in update_queue
  for (auto [_, b_weak] : m_renderQueue) {
    if (auto biome = b_weak.lock()) {
      constexpr auto count = CHUNK_COUNTX * CHUNK_COUNTZ;
      if (biome->m_chunksSetup.load(std::memory_order_acquire) == count) {
        biome->Update_queue(playerpos, VP);
      }
    }
  }
  // remove all expired
  bool isRemoved = false;
  for (auto it = m_renderQueue.begin(); it != m_renderQueue.end();) {
    auto sbptr = it->second.lock();
    auto pos = sbptr->m_biomePos;
    auto isPresent = m_biomes.get(pos.x / BIOME_LENGTH, pos.y / BIOME_HEIGHT,
                                  pos.z / BIOME_LENGTH);
    if (isPresent == nullptr) {
      it = m_renderQueue.erase(it);
      isRemoved = true;
    } else {
      ++it;
    }
  }

  // Mark for removal
  if (isRemoved) {
    {
      std::lock_guard<std::mutex> lock(m_setupMutex);
      m_jobQueue.emplace(0, 0, 0, glm::dvec3{}, true);
    }
    m_setupCv.notify_one();
  }
}

void World::MarkBiomesReadyForPass1() {
  auto neighborBiomes =
      [this](glm::ivec3 vec) -> std::array<std::weak_ptr<Biome>, 4> {
    std::weak_ptr<Biome> left, front, right, back;
    vec += glm::ivec3(HALF_BLOCK_SIZE);
    left = get_biome_by_center(vec + glm::ivec3(BIOME_LENGTH, 0, 0));
    front = get_biome_by_center(vec + glm::ivec3(0, 0, BIOME_LENGTH));
    right = get_biome_by_center(vec - glm::ivec3(BIOME_LENGTH, 0, 0));
    back = get_biome_by_center(vec - glm::ivec3(0, 0, BIOME_LENGTH));
    return {left, front, right, back};
  };
  // Go through each biome once
  uint32_t m_bindQueueSize = m_bindQueue.size();
  while (m_bindQueueSize--) {
    auto &[run, w_biome] = m_bindQueue.front();
    // If biome is null return early
    auto biome = w_biome.lock();

    if (!biome) {
      m_bindQueue.pop();
      continue;
    }

    if (biome->m_RenderIter.load() >= BIOMESTATUS::REFRESH) {
      // Biome already done with Pass 1
      return;
    }

    constexpr auto count = CHUNK_COUNTZ * CHUNK_COUNTX;
    bool isReady =
        biome->m_chunksSetup.load(std::memory_order_acquire) == count;
    // All the chunks belonging to biome are done with setup
    m_bindQueue.pop();
    if (isReady) {
      // Mark these chunks ready for rendering
      for (int i = 0; i < CHUNK_COUNTX; i++) {
        for (int j = 0; j < CHUNK_COUNTZ; j++) {
          auto chunk = biome->m_chunks[i][j];
          // Setup and update vaos for pass 1
          if (biome->m_Run == 0) {
            chunk->SetupVertexObjects();
          }
          chunk->UpdateVertexObjects();
          biome->m_renderQueue[chunk->m_id] = std::weak_ptr<Chunk>(chunk);
        }
      }
      auto neighbors = neighborBiomes(biome->m_biomePos);
      auto check = [](std::weak_ptr<Biome> bptr) {
        constexpr auto count = CHUNK_COUNTZ * CHUNK_COUNTX;
        auto sbptr = bptr.lock();
        if (!sbptr)
          return false;
        return sbptr->m_chunksSetup.load(std::memory_order_acquire) == count;
      };

      bool allAvailable =
          std::all_of(neighbors.begin(), neighbors.end(), check);
      // Pass Biome for interchunk walls removal
      m_rerenderQueue.push({run, biome});
      // Neighbor biomes available no need to rerender again
      assert(biome->m_RenderIter.load() == BIOMESTATUS::SETUP);
      if (allAvailable) {
        biome->m_RenderIter.store(BIOMESTATUS::FINAL);
      } else {
        m_waitingQueue.push({run, biome});
        biome->m_RenderIter.store(BIOMESTATUS::REFRESH);
      }
    } else {
      m_bindQueue.push({run, biome});
      continue;
    }
  }
}

void World::MarkBiomesReadyForPass2() {
  // Go through each biome
  uint32_t t_bindQueueSize = m_bindQueue.size();
  while (t_bindQueueSize--) {
    auto [run, w_biome] = m_bindQueue.front();

    auto biome = w_biome.lock();
    // If biome is null return early
    if (!biome) {
      m_bindQueue.pop();
      continue;
    }

    if (biome->m_RenderIter.load() <= BIOMESTATUS::SETUP) {
      // Biome not done with Pass 1
      return;
    }

    if (biome->m_RenderIter.load() == BIOMESTATUS::WAITING) {
      // Biome still in waiting state once done will be availble here again
      m_bindQueue.pop();
      continue;
    }
    assert(biome->m_RenderIter.load() == BIOMESTATUS::FINAL ||
           biome->m_RenderIter.load() == BIOMESTATUS::REFRESH);

    constexpr auto count = CHUNK_COUNTZ * CHUNK_COUNTX;
    auto ready1 = biome->m_chunksRerendered.load(std::memory_order_acquire);
    auto ready2 = biome->m_chunksFinished.load(std::memory_order_acquire);
    bool isRefresh = biome->m_RenderIter.load() == BIOMESTATUS::REFRESH;
    bool isFinal = biome->m_RenderIter.load() == BIOMESTATUS::FINAL;
    bool isReady =
        ((ready1 == count) && isRefresh) || ((ready2 == count) && isFinal);

    m_bindQueue.pop();
    if (isReady) {
      for (int i = 0; i < CHUNK_COUNTX; i++) {
        for (int j = 0; j < CHUNK_COUNTZ; j++) {
          auto chunk = biome->m_chunks[i][j];
          chunk->UpdateVertexObjects();
        }
      }
    } else {
      m_bindQueue.push({run, biome});
      continue;
    }
  }
}

void World::MarkBiomesReadyForBoundaryRemoval() {
  auto neighborBiomes =
      [this](glm::ivec3 vec) -> std::array<std::weak_ptr<Biome>, 4> {
    std::weak_ptr<Biome> left, front, right, back;
    vec += glm::ivec3(HALF_BLOCK_SIZE);
    left = get_biome_by_center(vec + glm::ivec3(BIOME_LENGTH, 0, 0));
    front = get_biome_by_center(vec + glm::ivec3(0, 0, BIOME_LENGTH));
    right = get_biome_by_center(vec - glm::ivec3(BIOME_LENGTH, 0, 0));
    back = get_biome_by_center(vec - glm::ivec3(0, 0, BIOME_LENGTH));
    return {left, front, right, back};
  };

  uint32_t t_waitingQueueSize = m_waitingQueue.size();
  while (t_waitingQueueSize--) {
    auto &[run, w_biome] = m_waitingQueue.front();

    auto biome = w_biome.lock();
    // If biome is null return early
    if (!biome) {
      m_waitingQueue.pop();
      continue;
    }

    constexpr auto count = CHUNK_COUNTZ * CHUNK_COUNTX;
    auto neighbors = neighborBiomes(biome->m_biomePos);
    auto check = [](std::weak_ptr<Biome> bptr) {
      constexpr auto count = CHUNK_COUNTZ * CHUNK_COUNTX;
      auto sbptr = bptr.lock();
      if (!sbptr)
        return false;
      return sbptr->m_chunksSetup.load(std::memory_order_acquire) == count;
    };

    bool all_available = std::all_of(neighbors.begin(), neighbors.end(), check);
    m_waitingQueue.pop();
    bool donewithPass2 =
        biome->m_chunksRerendered.load(std::memory_order_acquire) == count;

    assert(biome->m_RenderIter.load() == BIOMESTATUS::WAITING ||
           biome->m_RenderIter.load() == BIOMESTATUS::REFRESH);
    if (all_available && donewithPass2) {
      // All biomes available reschedule removal
      m_rerenderQueue.push({run, biome});
      biome->m_RenderIter.store(BIOMESTATUS::FINAL);
    } else {
      m_waitingQueue.push({run, biome});
      if (donewithPass2) {
        biome->m_RenderIter.store(BIOMESTATUS::WAITING);
      }
      continue;
    }
  }
}

void World::save_model(std::shared_ptr<Chunk> chunk, std::string name) {
  std::vector<glm::ivec3> ref_array;
  for (int i = 0; i < CHUNK_BLOCK_COUNT; i++) {
    for (int k = 0; k < CHUNK_BLOCK_COUNT; k++) {
      for (int j = 0; j < CHUNK_BLOCK_COUNT; j++) {
        auto blk = chunk->m_blocks[i][j][k];
        if (blk.is_ref()) {
          ref_array.push_back({i, j, k});
        }
      }
    }
  }

  if (ref_array.size() != 2) {
    std::cerr << "Reference blocks are not 2, ignoring model save: "
              << ref_array.size() << '\n';
  } else {
    std::cout << "Saving Model\n";
    std::ofstream save_model("models/" + name + ".bin",
                             std::ios::binary | std::ios::trunc);
    int countx = (ref_array[1].x - ref_array[0].x + 1),
        county = (ref_array[1].y - ref_array[0].y + 1),
        countz = (ref_array[1].z - ref_array[0].z + 1);

    std::cout << "Saved " << countx * county * countz << " blocks\n";
    save_model.write(reinterpret_cast<char *>(&countx), sizeof(countx));
    save_model.write(reinterpret_cast<char *>(&county), sizeof(county));
    save_model.write(reinterpret_cast<char *>(&countz), sizeof(countz));
    for (int i = ref_array[0].x; i <= ref_array[1].x; i++) {
      for (int k = ref_array[0].z; k <= ref_array[1].z; k++) {
        for (int j = ref_array[0].y; j <= ref_array[1].y; j++) {
          auto blk = chunk->m_blocks[i][j][k];
          if (blk.is_ref())
            continue;
          save_model.write(reinterpret_cast<char *>(&blk.blmask),
                           sizeof(GLuint));
        }
      }
    }
  }
}

void World::load_model(glm::ivec3 pos, std::string model, bool refresh_chunk) {
  // Read saved models files if any
  std::ifstream input_model_bin_file(model, std::ios::binary);
  if (!input_model_bin_file) {
    std::cerr << "Failed to open saved model file.\n";
    return;
  }

  int countx = 0, county = 0, countz = 0;
  input_model_bin_file.read(reinterpret_cast<char *>(&countx), sizeof(countx));
  input_model_bin_file.read(reinterpret_cast<char *>(&county), sizeof(county));
  input_model_bin_file.read(reinterpret_cast<char *>(&countz), sizeof(countz));
  if (auto biome = get_biome_by_center(pos).lock())
    biome->m_dirtyBit = 1;
  else {
    std::cout << "Biome is null\n";
    return;
  }
  std::cout << "Loaded " << countx * county * countz << " blocks\n";
  if (county * BLOCK_SIZE + pos.y > BIOME_COUNTY * BIOME_HEIGHT) {
    std::cerr << "MODEL to big to fitin, select a lower altitude\n";
    return;
  }
  bool left = false, back = false, right = false, front = false;
  for (int i = 0; i < countx; i++) {
    for (int k = 0; k < countz; k++) {
      for (int j = 0; j < county; j++) {
        // Skip the two ref blocks
        if ((i == 0 && j == 0 && k == 0) ||
            (i == countx - 1 && j == county - 1 && k == countz - 1))
          continue;
        Block block;
        input_model_bin_file.read(reinterpret_cast<char *>(&block),
                                  sizeof(block));
        // if (!block.isSolid()) continue;
        glm::dvec3 ptr = {pos.x + i * BLOCK_SIZE, pos.y + j * BLOCK_SIZE,
                          pos.z + k * BLOCK_SIZE}; // 63 1 63

        if (auto chunk = get_chunk_by_center(ptr).lock()) {
          chunk->m_dirtyBit = 1;
        } else {
          std::cout << "Chunk is null or deleted\n";
          return;
        }
        GLuint preserve_mask = ((1 << 15) - 1);
        // binary: 0000...01111111111111111 (15 bits set)
        GLuint overwrite_mask = ~preserve_mask;

        auto idx_x =
            (pos.x / static_cast<int>(BLOCK_SIZE) + i) % CHUNK_BLOCK_COUNT;
        auto idx_y =
            (pos.y / static_cast<int>(BLOCK_SIZE) + j) % CHUNK_BLOCK_COUNT;
        auto idx_z =
            (pos.z / static_cast<int>(BLOCK_SIZE) + k) % CHUNK_BLOCK_COUNT;

        if (idx_x == 0) {
          right = true;
        } else if (idx_x == CHUNK_BLOCK_COUNT - 1) {
          left = true;
        } else if (idx_z == 0) {
          front = true;
        } else if (idx_z == CHUNK_BLOCK_COUNT - 1) {
          back = true;
        }

        if (auto chunk = get_chunk_by_center(ptr).lock()) {

          Block &existing = chunk->m_blocks[idx_x][idx_y][idx_z];

          // Keep lower 15 bits of existing, replace rest from new
          existing.blmask = (existing.blmask & preserve_mask) |
                            (block.blmask & overwrite_mask);
        }
      }
    }
  }
  if (refresh_chunk) {
    RefreshChunks(pos, left, back, right, front);
  }
}

// Saving Scope
void World::save(std::string _save_file) {
  std::string path = "save/" + _save_file + ".bin";
  std::ofstream save_file(path.c_str(), std::ios::binary | std::ios::trunc);
  // Save All the dirty chunks

  auto save_chunk = [this](std::shared_ptr<Biome> biome) {
    for (int k = 0; k < CHUNK_COUNTZ; k++) {
      for (int l = 0; l < CHUNK_COUNTX; l++) {
        auto chunk = biome->m_chunks[k][l];
        if (!chunk || !chunk->m_dirtyBit)
          continue;
        m_saveMap[chunk->m_saveId] = chunk;
      }
    }
  };

  {
    std::shared_lock<std::shared_mutex> lock(m_biomes.biome_mutex);
    for (auto [_, biome] : m_biomes.BiomeMap) {
      if (!biome || !biome->m_dirtyBit)
        continue;
      save_chunk(biome);
    }
  }
  int count = m_saveMap.size();
  save_file.write(reinterpret_cast<char *>(&count), sizeof(count));
  std::cout << "Saving " << count << " chunks\n";
  for (auto [id, chunk_weak] : m_saveMap) {
    std::cout << "Saving chunk with ID: " << id << '\n';
    if (auto chunk = chunk_weak.lock()) {
      chunk->Serialize(save_file);
    }
  }
  std::cout << "Game Saved\n";
}

int World::getSeed() { return m_seed; }

// Client side
void World::handleNetworkRequest(WorldState &wst) {
  if (wst.model_idx) {
    load_model(wst.blockpos, "models/" + MODEL_ARRAY[wst.model_idx] + ".bin");
  } else if (auto block = get_block_by_center(wst.blockpos)) {
    auto _data = wst.blk;
    block->blmask = _data;
  }
  std::lock_guard<std::mutex> l{m};
  refreshq.push(wst.blockpos);
}

bool World::Valid(WorldState &wst) { return true; }

// Server/Client side
std::string World::get_state(WorldState &ws) {
  auto chunk = get_chunk_by_center(ws.blockpos);
  auto _data = ws.blk;
  auto block = get_block_by_center(ws.blockpos);

  State st{};
  st.ts = glfwGetTime();
  WorldState wst{};
  wst.blockpos = ws.blockpos;
  wst.model_idx = ws.model_idx;
  if (block) {
    wst.blk = block->blmask;
  }
  st._data = wst;

  std::string out(sizeof(State), '\0');
  std::memcpy(out.data(), &st, sizeof(State));
  return out;
}

// Server
std::shared_ptr<std::string>
World::handle_client_input(const std::string &msg) {
  State st{};
  std::memcpy(&st, msg.data(), sizeof(State));

  WorldState wst = std::get<WorldState>(st._data);
  if (!Valid(wst)) { // if not valid return last state
    st.enforce = true;
    return std::make_shared<std::string>(get_state(wst));
  }

  auto chunk = get_chunk_by_center(wst.blockpos);
  if (wst.model_idx != 0) {
    load_model(wst.blockpos, "models/" + MODEL_ARRAY[wst.model_idx] + ".bin");
    st.enforce = true;
    std::string resp = get_state(wst);
#ifdef BUILD_SERVER
    world_operations.push_back(resp);
#endif
    return std::make_shared<std::string>(std::move(resp));
  }
  // if verified return new state

  // TODO: Update state on server
#ifdef BUILD_SERVER
  world_operations.push_back(msg);
#endif
  return std::make_shared<std::string>(msg);
}

void World::RefreshChunks(glm::ivec3 rayhitcord, bool left, bool back,
                          bool right, bool front) {
  auto vec = rayhitcord;

  // get neighbouring chunks
  auto get_neighbors =
      [this](glm::ivec3 vec) -> std::array<std::weak_ptr<Chunk>, 8> {
    std::weak_ptr<Chunk> left, leftback, front, rightback, right, rightfront,
        back, leftfront;
    left = get_chunk_by_center(vec + glm::ivec3(CHUNK_LENGTH, 0, 0));
    front = get_chunk_by_center(vec + glm::ivec3(0, 0, CHUNK_LENGTH));
    right = get_chunk_by_center(vec - glm::ivec3(CHUNK_LENGTH, 0, 0));
    back = get_chunk_by_center(vec - glm::ivec3(0, 0, CHUNK_LENGTH));
    leftback =
        get_chunk_by_center(vec + glm::ivec3(CHUNK_LENGTH, 0, CHUNK_LENGTH));
    rightback =
        get_chunk_by_center(vec + glm::ivec3(-CHUNK_LENGTH, 0, CHUNK_LENGTH));
    rightfront =
        get_chunk_by_center(vec + glm::ivec3(-CHUNK_LENGTH, 0, -CHUNK_LENGTH));
    leftfront =
        get_chunk_by_center(vec + glm::ivec3(CHUNK_LENGTH, 0, -CHUNK_LENGTH));
    return {left,     front,     right,      back,
            leftback, rightback, rightfront, leftfront};
  };

  int cordz = vec.z % CHUNK_LENGTH;
  int cordx = vec.x % CHUNK_LENGTH;

  if (auto chunk = get_chunk_by_center(vec).lock()) {
    // If last block update adjacent chunk
    chunk->Render(0, true, nullptr, nullptr, nullptr, nullptr);
  }
  auto neighchunks = get_neighbors(vec);
  if (front || (cordz == CHUNK_LENGTH - 1)) { // frnt
    front = true;
    auto neighneighchunks = get_neighbors(vec + glm::ivec3(0, 0, CHUNK_LENGTH));
    if (auto n1 = neighchunks[1].lock()) {
      std::cout << "[FRONT] Updating neighbouring chunk\n";
      n1->Render(0, true, nullptr, nullptr, nullptr, nullptr);
      n1->Render(0, false, neighneighchunks[0].lock(),
                 neighneighchunks[1].lock(), neighneighchunks[2].lock(),
                 neighneighchunks[3].lock());
    }
  }

  if (left || (cordx == CHUNK_LENGTH - 1)) { // left
    left = true;
    auto neighneighchunks = get_neighbors(vec + glm::ivec3(CHUNK_LENGTH, 0, 0));
    if (auto n0 = neighchunks[0].lock()) {
      std::cout << "[LEFT] Updating neighbouring chunk\n";
      n0->Render(0, true, nullptr, nullptr, nullptr, nullptr);
      n0->Render(0, false, neighneighchunks[0].lock(),
                 neighneighchunks[1].lock(), neighneighchunks[2].lock(),
                 neighneighchunks[3].lock());
    }
  }

  if (back || cordz == 1) { // back
    back = true;
    auto neighneighchunks = get_neighbors(vec - glm::ivec3(0, 0, CHUNK_LENGTH));
    if (auto n3 = neighchunks[3].lock()) {
      std::cout << "[BACK] Updating neighbouring chunk\n";
      n3->Render(0, true, nullptr, nullptr, nullptr, nullptr);
      n3->Render(0, false, neighneighchunks[0].lock(),
                 neighneighchunks[1].lock(), neighneighchunks[2].lock(),
                 neighneighchunks[3].lock());
    }
  }

  if (right || cordx == 1) { // right
    right = true;
    auto neighneighchunks = get_neighbors(vec - glm::ivec3(CHUNK_LENGTH, 0, 0));
    if (auto n2 = neighchunks[2].lock()) {
      std::cout << "[RIGHT] Updating neighbouring chunk\n";
      n2->Render(0, true, nullptr, nullptr, nullptr, nullptr);
      n2->Render(0, false, neighneighchunks[0].lock(),
                 neighneighchunks[1].lock(), neighneighchunks[2].lock(),
                 neighneighchunks[3].lock());
    }
  }

  if (left && back) {
    auto neighneighchunks =
        get_neighbors(vec + glm::ivec3(CHUNK_LENGTH, 0, CHUNK_LENGTH));
    if (auto n4 = neighchunks[4].lock()) {
      std::cout << "[RIGHT] Updating neighbouring chunk\n";
      n4->Render(0, true, nullptr, nullptr, nullptr, nullptr);
      n4->Render(0, false, neighneighchunks[0].lock(),
                 neighneighchunks[1].lock(), neighneighchunks[2].lock(),
                 neighneighchunks[3].lock());
    }
  }

  if (right && back) {
    auto neighneighchunks =
        get_neighbors(vec + glm::ivec3(-CHUNK_LENGTH, 0, CHUNK_LENGTH));
    if (auto n5 = neighchunks[5].lock()) {
      std::cout << "[RIGHT] Updating neighbouring chunk\n";
      n5->Render(0, true, nullptr, nullptr, nullptr, nullptr);
      n5->Render(0, false, neighneighchunks[0].lock(),
                 neighneighchunks[1].lock(), neighneighchunks[2].lock(),
                 neighneighchunks[3].lock());
    }
  }

  if (right && front) {
    auto neighneighchunks =
        get_neighbors(vec + glm::ivec3(-CHUNK_LENGTH, 0, -CHUNK_LENGTH));
    if (auto n6 = neighchunks[6].lock()) {
      std::cout << "[RIGHT] Updating neighbouring chunk\n";
      n6->Render(0, true, nullptr, nullptr, nullptr, nullptr);
      n6->Render(0, false, neighneighchunks[0].lock(),
                 neighneighchunks[1].lock(), neighneighchunks[2].lock(),
                 neighneighchunks[3].lock());
    }
  }

  if (left && front) {
    auto neighneighchunks =
        get_neighbors(vec + glm::ivec3(CHUNK_LENGTH, 0, -CHUNK_LENGTH));
    if (auto n7 = neighchunks[7].lock()) {
      std::cout << "[RIGHT] Updating neighbouring chunk\n";
      n7->Render(0, true, nullptr, nullptr, nullptr, nullptr);
      n7->Render(0, false, neighneighchunks[0].lock(),
                 neighneighchunks[1].lock(), neighneighchunks[2].lock(),
                 neighneighchunks[3].lock());
    }
  }

  if (auto chunk = get_chunk_by_center(vec).lock()) {
    chunk->Render(0, false, neighchunks[0].lock(), neighchunks[1].lock(),
                  neighchunks[2].lock(), neighchunks[3].lock());
  }
}

WEATHER World::getWeather() { return m_weather; }

void World::setWeather(WEATHER weather) { m_weather = weather; }
