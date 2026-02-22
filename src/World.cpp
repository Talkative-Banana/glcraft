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
    std::cout << "Loaded a chunk with ID: " << chunk.save_id << std::endl;
    load_map[chunk.save_id] = std::move(chunk);
  }
};

Block *World::get_block_by_center(const glm::ivec3 &pos) {
  // get the biome
  // get x and z cords
  glm::ivec3 pos_cpy = pos - glm::ivec3(HALF_BLOCK_SIZE);
  int x = pos_cpy.x / (BLOCK_SIZE), y = pos_cpy.y / (BLOCK_SIZE),
      z = pos_cpy.z / (BLOCK_SIZE);

  if (pos_cpy.x < 0 || pos_cpy.y < 0 || pos_cpy.z < 0)
    return nullptr;

  if (y < 0 || y >= CHUNK_BLOCK_COUNT)
    return nullptr;

  // get the biome
  int biomex = x / (CHUNK_BLOCK_COUNT * CHUNK_COUNTX),
      biomez = z / (CHUNK_BLOCK_COUNT * CHUNK_COUNTZ);

  if (biomex >= BIOME_COUNTX || biomez >= BIOME_COUNTZ)
    return nullptr;

  auto biome = biomes[biomex][biomez];
  if (!biome)
    return nullptr;
  // get the chunk

  // get the chunk
  int chunkx = (x / CHUNK_BLOCK_COUNT) % CHUNK_COUNTX,
      chunkz = (z / CHUNK_BLOCK_COUNT) % CHUNK_COUNTZ;

  auto chunk = biome->chunks[chunkx][chunkz];

  if (!chunk)
    return nullptr;
  // get the block
  if (y >= CHUNK_BLOCK_COUNT)
    return nullptr;
  auto &block = chunk->blocks[x % CHUNK_BLOCK_COUNT][y][z % CHUNK_BLOCK_COUNT];
  return &block;
}

std::shared_ptr<Chunk> World::get_chunk_by_center(const glm::ivec3 &pos) {
  // get the biome
  // get x and z cords
  glm::ivec3 pos_cpy = pos - glm::ivec3(HALF_BLOCK_SIZE);
  int x = pos_cpy.x / (BLOCK_SIZE), z = pos_cpy.z / (BLOCK_SIZE);

  if (pos_cpy.x < 0 || pos_cpy.z < 0)
    return nullptr;

  // get the biome
  int biomex = x / (CHUNK_BLOCK_COUNT * CHUNK_COUNTX),
      biomez = z / (CHUNK_BLOCK_COUNT * CHUNK_COUNTZ);

  if (biomex >= BIOME_COUNTX || biomez >= BIOME_COUNTZ)
    return nullptr;

  auto biome = biomes[biomex][biomez];
  if (!biome)
    return nullptr;
  // get the chunk

  // get the chunk
  int chunkx = (x / CHUNK_BLOCK_COUNT) % CHUNK_COUNTX,
      chunkz = (z / CHUNK_BLOCK_COUNT) % CHUNK_COUNTZ;

  auto chunk = biome->chunks[chunkx][chunkz];

  return chunk ? chunk : nullptr;
}

std::shared_ptr<Biome> World::get_biome_by_center(const glm::ivec3 &pos) {
  // get the biome
  // get x and z cords
  glm::ivec3 pos_cpy = pos - glm::ivec3(HALF_BLOCK_SIZE);
  int x = pos_cpy.x / (BLOCK_SIZE), z = pos_cpy.z / (BLOCK_SIZE);

  if (pos_cpy.x < 0 || pos_cpy.z < 0)
    return nullptr;

  // get the biome
  int biomex = x / (CHUNK_BLOCK_COUNT * CHUNK_COUNTX),
      biomez = z / (CHUNK_BLOCK_COUNT * CHUNK_COUNTZ);

  if (biomex >= BIOME_COUNTX || biomez >= BIOME_COUNTZ)
    return nullptr;

  auto biome = biomes[biomex][biomez];
  if (!biome)
    return nullptr;

  return biome ? biome : nullptr;
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
  while (running) {
    std::unique_lock<std::mutex> lock(setup_mutex);
    setup_cv.wait(lock, [this] { return !job_queue.empty() || !running; });

    if (!running)
      break;

    auto [i, j, pos] = job_queue.front();
    job_queue.pop();
    lock.unlock();

    // heavy work outside lock
    int idx = BIOME_COUNTX * i + j;
    if (biomes[i][j])
      continue;
    auto biome = std::make_shared<Biome>(0, pos, true);

    {
      std::lock_guard<std::mutex> g(setup_mutex);
      biomes[i][j] = biome;
      setup_queue.push(biome);
    }
  }
}

void World::SetupWorld(glm::vec3 playerpos) {
  // Do not set up for all the biomes
  for (int i = 0; i < BIOME_COUNTX; i++) {
    for (int j = 0; j < BIOME_COUNTZ; j++) {
      int idx = BIOME_COUNTX * i + j;
      float common = CHUNK_COUNTX * CHUNK_BLOCK_COUNT * BLOCK_SIZE;
      glm::ivec3 biome_pos = glm::ivec3(common * i, 0, common * j);
      int X = biome_pos.x - playerpos.x, Z = biome_pos.z - playerpos.z;
      auto biome = biomes[i][j];
      if (((X <= RENDER_DISTANCE) && (Z <= RENDER_DISTANCE)) &&
          (job_scheduled.find(idx) == job_scheduled.end())) {
        // Costly move it to a seprate thread
        {
          std::lock_guard<std::mutex> lock(setup_mutex);
          job_queue.emplace(i, j, m_worldpos + biome_pos);
          job_scheduled.insert(idx);
        }
        setup_cv.notify_one();
      } else if (Z > RENDER_DISTANCE) {
        break;
      } else if (X > RENDER_DISTANCE) {
        i = BIOME_COUNTX;
        break;
      }
    }
  }
}

void World::RenderWorld(bool firstRun) {
  std::lock_guard<std::mutex> lock(setup_mutex);
  if (firstRun) {
    while (!setup_queue.empty()) {
      auto b = setup_queue.front();
      setup_queue.pop();
      b->RenderBiome(true); // firstRun
      b->isrerenderiter = false;
      if (render_queue.find(b) == render_queue.end())
        render_queue.insert(b);
    }
  } else {
    while (!rerender_queue.empty()) {
      auto b = rerender_queue.front();
      rerender_queue.pop();
      b->RenderBiome(false); // ReRun
      b->isrerenderiter = true;
      if (render_queue.find(b) == render_queue.end())
        render_queue.insert(b);
    }
  }
}

void World::Draw(OBJ_TYPE type) {
  // Do not render all the chunks just what biome wants to using its
  // render_queue
  for (auto biome : render_queue) {
    if (!biome) {
      std::cerr << "[ERROR] World::Draw biome is null\n";
      continue;
    }
    if (biome->chunks_ready.load(std::memory_order_acquire) >=
        CHUNK_COUNTX * CHUNK_COUNTZ) {
      biome->Draw(type);
    }
  }
}

void World::Update_queue(glm::vec3 playerpos, glm::mat4 VP) {
  for (auto biome : render_queue) {
    if (!biome) {
      std::cerr << "[ERROR] World::Update_queue: biome is null\n";
      continue;
    }
    if (biome->chunks_ready.load(std::memory_order_acquire) >=
        CHUNK_COUNTX * CHUNK_COUNTZ) {
      biome->Update_queue(playerpos, VP);
    }
  }
}

void World::DoBindTask(bool firstRun) {
  while (!bind_queue.empty()) {
    bool flag = false;
    std::shared_ptr<Biome> biome;
    {
      std::lock_guard<std::mutex> lock(biome_mutex);
      biome = bind_queue.front();
      // If biome is null return early
      if (!biome)
        return;
      if (firstRun && biome->isrerenderiter) {
        return;
      }

      if (biome->isrerenderiter) {
        int expected = 32;
        if (biome->chunks_ready.compare_exchange_strong(expected, 16)) {
          flag = true;
        }
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
          biome->render_queue.insert(chunk);
        }
      }

      // Have a way to check if neighbor chunks are loaded before rerendering
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
  auto biome = get_biome_by_center(pos);
  if (biome)
    biome->dirtybit = 1;
  else {
    std::cout << "Biome is null\n";
    return;
  }
  std::cout << "Loaded " << countx * county * countz << " blocks\n";
  if (county * BLOCK_SIZE + pos.y > CHUNK_BLOCK_COUNT * BLOCK_SIZE) {
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
        auto chunk =
            get_chunk_by_center({pos.x + i * BLOCK_SIZE, pos.y + j * BLOCK_SIZE,
                                 pos.z + k * BLOCK_SIZE}); // 63 1 63
        if (chunk) {
          chunk->dirtybit = 1;
        } else {
          std::cout << "Chunk is null\n";
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
        Block &existing = chunk->blocks[idx_x][idx_y][idx_z];

        // Keep lower 15 bits of existing, replace rest from new
        existing.blmask =
            (existing.blmask & preserve_mask) | (block.blmask & overwrite_mask);
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
  for (int i = 0; i < BIOME_COUNTZ; i++) {
    for (int j = 0; j < BIOME_COUNTX; j++) {
      auto biome = biomes[i][j];
      if (!biome || !biome->dirtybit)
        continue;
      for (int k = 0; k < CHUNK_COUNTZ; k++) {
        for (int l = 0; l < CHUNK_COUNTX; l++) {
          auto chunk = biome->chunks[k][l];
          if (!chunk || !chunk->dirtybit)
            continue;
          std::cout << "Saving chunk with ID: " << i << " " << j << " " << k
                    << " " << l << std::endl;
          save_map[chunk->save_id] = chunk;
        }
      }
    }
  }
  int count = save_map.size();
  save_file.write(reinterpret_cast<char *>(&count), sizeof(count));
  std::cout << "Saving " << count << " chunks\n";
  for (auto [id, chunk] : save_map) {
    std::cout << "Saving chunk with ID: " << id << std::endl;
    chunk->Serialize(save_file);
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
      [this](glm::ivec3 vec) -> std::vector<std::shared_ptr<Chunk>> {
    std::shared_ptr<Chunk> left, leftback, front, rightback, right, rightfront,
        back, leftfront;
    left = get_chunk_by_center(
        vec +
        glm::ivec3(static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE), 0, 0));
    front = get_chunk_by_center(
        vec +
        glm::ivec3(0, 0, static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE)));
    right = get_chunk_by_center(
        vec -
        glm::ivec3(static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE), 0, 0));
    back = get_chunk_by_center(
        vec -
        glm::ivec3(0, 0, static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE)));
    leftback = get_chunk_by_center(
        vec + glm::ivec3(static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE), 0,
                         static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE)));
    rightback = get_chunk_by_center(
        vec + glm::ivec3(-static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE), 0,
                         static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE)));
    rightfront = get_chunk_by_center(
        vec + glm::ivec3(-static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE), 0,
                         -static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE)));
    leftfront = get_chunk_by_center(
        vec + glm::ivec3(static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE), 0,
                         -static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE)));
    return {left,     front,     right,      back,
            leftback, rightback, rightfront, leftfront};
  };

  int cordz = (vec.z % static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE));
  int cordx = (vec.x % static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE));

  auto chunk = get_chunk_by_center(vec);
  // If last block update adjacent chunk
  auto neighchunks = get_neighbors(vec);
  chunk->Render(0, true, nullptr, nullptr, nullptr, nullptr);

  if (front ||
      (cordz == static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE) - 1)) { // frnt
    front = true;
    auto neighneighchunks = get_neighbors(
        vec +
        glm::ivec3(0, 0, static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE)));
    if (neighchunks[1]) {
      std::cout << "[FRONT] Updating neighbouring chunk\n";
      neighchunks[1]->Render(0, true, nullptr, nullptr, nullptr, nullptr);
      neighchunks[1]->Render(0, false, neighneighchunks[0], neighneighchunks[1],
                             neighneighchunks[2], neighneighchunks[3]);
    }
  }

  if (left ||
      (cordx == static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE) - 1)) { // left
    left = true;
    auto neighneighchunks = get_neighbors(
        vec +
        glm::ivec3(static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE), 0, 0));
    if (neighchunks[0]) {
      std::cout << "[LEFT] Updating neighbouring chunk\n";
      neighchunks[0]->Render(0, true, nullptr, nullptr, nullptr, nullptr);
      neighchunks[0]->Render(0, false, neighneighchunks[0], neighneighchunks[1],
                             neighneighchunks[2], neighneighchunks[3]);
    }
  }

  if (back || cordz == 1) { // back
    back = true;
    auto neighneighchunks = get_neighbors(
        vec -
        glm::ivec3(0, 0, static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE)));
    if (neighchunks[3]) {
      std::cout << "[BACK] Updating neighbouring chunk\n";
      neighchunks[3]->Render(0, true, nullptr, nullptr, nullptr, nullptr);
      neighchunks[3]->Render(0, false, neighneighchunks[0], neighneighchunks[1],
                             neighneighchunks[2], neighneighchunks[3]);
    }
  }

  if (right || cordx == 1) { // right
    right = true;
    auto neighneighchunks = get_neighbors(
        vec -
        glm::ivec3(static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE), 0, 0));
    if (neighchunks[2]) {
      std::cout << "[RIGHT] Updating neighbouring chunk\n";
      neighchunks[2]->Render(0, true, nullptr, nullptr, nullptr, nullptr);
      neighchunks[2]->Render(0, false, neighneighchunks[0], neighneighchunks[1],
                             neighneighchunks[2], neighneighchunks[3]);
    }
  }

  if (left && back) {
    auto neighneighchunks = get_neighbors(
        vec + glm::ivec3(static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE), 0,
                         static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE)));
    if (neighchunks[4]) {
      std::cout << "[RIGHT] Updating neighbouring chunk\n";
      neighchunks[4]->Render(0, true, nullptr, nullptr, nullptr, nullptr);
      neighchunks[4]->Render(0, false, neighneighchunks[0], neighneighchunks[1],
                             neighneighchunks[2], neighneighchunks[3]);
    }
  }

  if (right && back) {
    auto neighneighchunks = get_neighbors(
        vec + glm::ivec3(-static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE), 0,
                         static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE)));
    if (neighchunks[5]) {
      std::cout << "[RIGHT] Updating neighbouring chunk\n";
      neighchunks[5]->Render(0, true, nullptr, nullptr, nullptr, nullptr);
      neighchunks[5]->Render(0, false, neighneighchunks[0], neighneighchunks[1],
                             neighneighchunks[2], neighneighchunks[3]);
    }
  }

  if (right && front) {
    auto neighneighchunks = get_neighbors(
        vec + glm::ivec3(-static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE), 0,
                         -static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE)));
    if (neighchunks[6]) {
      std::cout << "[RIGHT] Updating neighbouring chunk\n";
      neighchunks[6]->Render(0, true, nullptr, nullptr, nullptr, nullptr);
      neighchunks[6]->Render(0, false, neighneighchunks[0], neighneighchunks[1],
                             neighneighchunks[2], neighneighchunks[3]);
    }
  }

  if (left && front) {
    auto neighneighchunks = get_neighbors(
        vec + glm::ivec3(static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE), 0,
                         -static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE)));
    if (neighchunks[7]) {
      std::cout << "[RIGHT] Updating neighbouring chunk\n";
      neighchunks[7]->Render(0, true, nullptr, nullptr, nullptr, nullptr);
      neighchunks[7]->Render(0, false, neighneighchunks[0], neighneighchunks[1],
                             neighneighchunks[2], neighneighchunks[3]);
    }
  }

  chunk->Render(0, false, neighchunks[0], neighchunks[1], neighchunks[2],
                neighchunks[3]);
}
