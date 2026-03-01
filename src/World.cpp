#include "World.h"

#include <fstream>

std::mutex m;
std::queue<glm::ivec3> refreshq;

World::World(int seed, const glm::ivec3 &pos) : m_seed(seed), m_worldpos(pos) {
  // Read saved files if any
  worker = std::thread(&World::workerLoop, this);
  std::ifstream input_bin_file("save/ff1.bin", std::ios::binary);
  if (!input_bin_file) {
    std::cerr << "Failed to open save file.\n";
    return;
  }

  int count = 1;
  input_bin_file.read(reinterpret_cast<char *>(&count), sizeof(count));
  for (int i = 0; i < count; i++) {
    Chunk chunk;
    if (!chunk.Deserialize(input_bin_file))
      break;
    std::cout << "Loaded a chunk with ID: " << chunk.save_id << '\n';
    load_map.emplace(chunk.save_id, std::move(chunk));
  }
};

World::~World() {
  m_running = false;
  setup_cv.notify_all();

  if (worker.joinable()) {
    worker.join();
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

  auto biome = biomes.get(biomex, biomey, biomez);
  if (!biome)
    return nullptr;
  // get the chunk

  // get the chunk
  int chunkx = (x / CHUNK_BLOCK_COUNT) % CHUNK_COUNTX,
      chunkz = (z / CHUNK_BLOCK_COUNT) % CHUNK_COUNTZ;

  auto chunk = biome->chunks[chunkx][chunkz];

  if (!chunk)
    return nullptr;
  auto &block = chunk->blocks[x % CHUNK_BLOCK_COUNT][y % CHUNK_BLOCK_COUNT]
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

  auto biome = biomes.get(biomex, biomey, biomez);
  if (!biome)
    return std::weak_ptr<Chunk>{};

  // get the chunk
  int chunkx = (x / CHUNK_BLOCK_COUNT) % CHUNK_COUNTX,
      chunkz = (z / CHUNK_BLOCK_COUNT) % CHUNK_COUNTZ;

  auto chunk = biome->chunks[chunkx][chunkz];

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

  auto biome = biomes.get(biomex, biomey, biomez);
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
      std::unique_lock<std::mutex> lock(setup_mutex);
      setup_cv.wait(lock, [this] { return !job_queue.empty() || !m_running; });
      if (!m_running) {
        lock.unlock();
        break;
      }
      auto [i, j, k, pos, isremove] = job_queue.front();
      job_queue.pop();
      lock.unlock();

      if (isremove) {
        // Remove all of the unwanted biomes
        uint64_t id = biomes.cleartoRemove();
        if (id) {
          std::lock_guard<std::mutex> lck(setup_mutex);
          job_scheduled.erase(id);
          render_queue.erase(id);
        }
        continue; // Was a deletion request
      }
      std::string biomeId = std::to_string(biomes.index(i, k, j));
      std::string file_name = "save/tmp/" + biomeId + ".bin";
      // heavy work outside lock
      if (biomes.get(i, k, j)) {
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
          std::cout << "Loading chunk: " << chunk.save_id << '\n';
          load_map.emplace(chunk.save_id, std::move(chunk));
        }
      }
      auto biome = std::make_shared<Biome>(0, pos, true);

      {
        std::lock_guard<std::mutex> g(setup_mutex);
        biomes.set(i, k, j, biome);
        setup_queue.push(biome);
      }
    }
  }
}

void World::EnqueueVisibleBiomes(glm::dvec3 playerpos) {
  // Do not set up for all the biomes
  int player_k = playerpos.y / BIOME_HEIGHT;
  int player_j = playerpos.z / BIOME_LENGTH;
  int player_i = playerpos.x / BIOME_LENGTH;

  for (uint64_t k = std::min(player_k, BIOME_COUNTY - 1);
       k >= std::max(player_k - 2, 0); k--) {
    for (uint64_t i = std::max(0, player_i - 2);
         i <= std::min(player_i + 2, BIOME_COUNTX - 1); i++) {
      for (uint64_t j = std::max(0, player_j - 2);
           j <= std::min(player_j + 2, BIOME_COUNTZ - 1); j++) {
        uint64_t idx = k * (BIOME_COUNTX * BIOME_COUNTZ) + BIOME_COUNTX * i + j;

        glm::ivec3 biome_pos =
            glm::ivec3(BIOME_LENGTH * i, BIOME_HEIGHT * k, BIOME_LENGTH * j);
        bool insideX = abs(biome_pos.x - playerpos.x) <= RENDER_DISTANCE,
             insideZ = abs(biome_pos.z - playerpos.z) <= RENDER_DISTANCE,
             insideY = abs(biome_pos.y - playerpos.y) <= CHUNK_HEIGHT * 2;
        if (insideX && insideZ && insideY) {
          std::lock_guard<std::mutex> lock(setup_mutex);
          bool isPresent = job_scheduled.find(idx) != job_scheduled.end();
          // Costly move it to a seprate thread
          if (!isPresent) {
            job_queue.emplace(i, j, k, m_worldpos + biome_pos, false);
            job_scheduled.insert(idx);
          }
          setup_cv.notify_one();
        }
      }
    }
  }
}

void World::SetupBiomesPass1() {
  std::lock_guard<std::mutex> lock(setup_mutex);
  while (!setup_queue.empty()) {
    auto b_weak = setup_queue.front();
    setup_queue.pop();
    if (auto b = b_weak.lock()) {
      b->SetupBiome(true); // firstRun
      b->isrerenderiter = false;
      if (render_queue.find(b->m_id) == render_queue.end())
        render_queue[b->m_id] = b_weak;
    }
  }
}

void World::SetupBiomesPass2() {
  std::lock_guard<std::mutex> lock(setup_mutex);
  while (!rerender_queue.empty()) {
    auto b_weak = rerender_queue.front();
    rerender_queue.pop();
    if (auto b = b_weak.lock()) {
      b->SetupBiome(false); // ReRun
      b->isrerenderiter = true;
      if (render_queue.find(b->m_id) == render_queue.end())
        render_queue[b->m_id] = b_weak;
    }
  }
}

void World::Draw(OBJ_TYPE type, glm::dvec3 cameraPos) {
  // Do not render all the biomes just what world wants to using its
  // render_queue
  for (auto [_, b_weak] : render_queue) {
    if (auto biome = b_weak.lock()) {
      if (biome->chunks_ready.load(std::memory_order_acquire) >=
          CHUNK_COUNTX * CHUNK_COUNTZ) {
        biome->Draw(type, cameraPos);
      }
    }
  }
}

void World::Update_queue(glm::dvec3 playerpos, glm::dmat4 VP) {
  // Check for all the biomes in update_queue
  for (auto [_, b_weak] : render_queue) {
    if (auto biome = b_weak.lock()) {
      if (biome->chunks_ready.load(std::memory_order_acquire) >=
          CHUNK_COUNTX * CHUNK_COUNTZ) {
        biome->Update_queue(playerpos, VP);
      }
    }
  }

  {
    std::lock_guard<std::mutex> lock(setup_mutex);
    job_queue.emplace(0, 0, 0, glm::dvec3{}, true);
  }
}

void World::DoBindTask(bool firstRun) {
  std::lock_guard<std::mutex> lock(biome_mutex);
  while (!bind_queue.empty()) {
    auto biome = bind_queue.front().lock();
    bool flag = false;
    {
      // If biome is null return early
      if (!biome) {
        bind_queue.pop();
        break;
      }
      if (firstRun && biome->isrerenderiter) {
        return;
      }
    }

    if (biome->isrerenderiter) {
      int expected = 32;
      if (biome->chunks_ready.compare_exchange_strong(expected, 16)) {
        flag = true;
      }
    }
    if ((firstRun || flag) &&
        biome->chunks_ready.load(std::memory_order_acquire) ==
            CHUNK_COUNTZ * CHUNK_COUNTX) {
      bind_queue.pop();
      for (int i = 0; i < CHUNK_COUNTX; i++) {
        for (int j = 0; j < CHUNK_COUNTZ; j++) {
          auto chunk = biome->chunks[i][j];
          // OPAQUE PASS
          chunk->chunkva = std::make_unique<VertexArray>();
          chunk->chunkva->Bind();
          VertexBufferLayout layout;
          layout.Push(GL_UNSIGNED_INT, 1);
          chunk->chunkvb = std::make_unique<VertexBuffer>(
              chunk->cube_vertices.data(),
              chunk->cube_vertices.size() * sizeof(GLuint));
          chunk->chunkva->AddBuffer(*(chunk->chunkvb), layout);
          chunk->chunkib = std::make_unique<IndexBuffer>(
              chunk->cube_indices.data(), chunk->cube_indices.size());
          chunk->chunkib->Bind();
          chunk->chunkva->Unbind();

          // TRANSPARENT PASS
          chunk->chunkvatrans = std::make_unique<VertexArray>();
          chunk->chunkvatrans->Bind();
          VertexBufferLayout layouttrans;
          layouttrans.Push(GL_UNSIGNED_INT, 1);
          chunk->chunkvbtrans = std::make_unique<VertexBuffer>(
              chunk->cube_verticestrans.data(),
              chunk->cube_verticestrans.size() * sizeof(GLuint));
          chunk->chunkvatrans->AddBuffer(*(chunk->chunkvbtrans), layouttrans);
          chunk->chunkibtrans = std::make_unique<IndexBuffer>(
              chunk->cube_indicestrans.data(), chunk->cube_indicestrans.size());

          chunk->chunkibtrans->Bind();
          chunk->chunkvatrans->Unbind();
          biome->render_queue[chunk->id] = std::weak_ptr<Chunk>(chunk);
        }
      }

      // Have a way to check if neighbor chunks are loaded before
      // rerendering
      if (firstRun) {
        rerender_queue.push(biome);
      }
    } else {
      return;
    }
  }
}

void World::save_model(std::shared_ptr<Chunk> chunk, std::string name) {
  std::vector<glm::ivec3> ref_array;
  for (int i = 0; i < CHUNK_BLOCK_COUNT; i++) {
    for (int k = 0; k < CHUNK_BLOCK_COUNT; k++) {
      for (int j = 0; j < CHUNK_BLOCK_COUNT; j++) {
        auto blk = chunk->blocks[i][j][k];
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
          auto blk = chunk->blocks[i][j][k];
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
    biome->dirtybit = 1;
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
          chunk->dirtybit = 1;
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

          Block &existing = chunk->blocks[idx_x][idx_y][idx_z];

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
        auto chunk = biome->chunks[k][l];
        if (!chunk || !chunk->dirtybit)
          continue;
        save_map[chunk->save_id] = chunk;
      }
    }
  };

  for (auto [_, biome] : biomes.BiomeMap) {
    if (!biome || !biome->dirtybit)
      continue;
    save_chunk(biome);
  }
  int count = save_map.size();
  save_file.write(reinterpret_cast<char *>(&count), sizeof(count));
  std::cout << "Saving " << count << " chunks\n";
  for (auto [id, chunk_weak] : save_map) {
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
      [this](glm::ivec3 vec) -> std::vector<std::weak_ptr<Chunk>> {
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
