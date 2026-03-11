#include "Biome.h"

#include <fstream>

#include "Renderer.h"
#include "World.h"

extern std::unique_ptr<World> world;

void Biome::allocate_chunks(bool isNew) {
  for (int i = 0; i < CHUNK_COUNTX; i++) {
    for (int j = 0; j < CHUNK_COUNTZ; j++) {
      int idx = CHUNK_COUNTX * i + j;
      glm::ivec3 t_chunkpos = glm::ivec3(i, 0, j);
      bool t_db = m_displayBiome;
      glm::ivec3 t_bp = m_biomePos;
      if (isNew) {
        m_chunks[i][j] =
            std::make_shared<Chunk>(idx, t_bp, t_chunkpos, t_db, m_type);
      } else {
        m_chunks[i][j]->RecycleChunk(idx, t_bp, t_chunkpos, t_db, m_type);
      }
    }
  }
}

void Biome::setup_chunks(bool firstRun) {
  for (int i = 0; i < CHUNK_COUNTX; i++) {
    for (int j = 0; j < CHUNK_COUNTZ; j++) {
      if (!m_running) {
        return;
      }
      auto _chunk = m_chunks[i][j];
      if (firstRun) {
        _chunk->Render(1, firstRun, nullptr, nullptr, nullptr, nullptr);
      } else {
        if (i == 0 || i == CHUNK_COUNTX - 1 || j == 0 ||
            j == CHUNK_COUNTZ - 1) {
          // Have to check neigbouring biome
          // get the center of chunks 1st block
          glm::ivec3 p = _chunk->m_chunkPos + glm::ivec3(HALF_BLOCK_SIZE);
          auto get_neighbors =
              [](glm::ivec3 vec) -> std::vector<std::weak_ptr<Chunk>> {
            std::weak_ptr<Chunk> left, front, right, back;
            left = world->get_chunk_by_center(
                vec + glm::ivec3(static_cast<int>(CHUNK_LENGTH), 0, 0));
            front = world->get_chunk_by_center(
                vec + glm::ivec3(0, 0, static_cast<int>(CHUNK_LENGTH)));
            right = world->get_chunk_by_center(
                vec - glm::ivec3(static_cast<int>(CHUNK_LENGTH), 0, 0));
            back = world->get_chunk_by_center(
                vec - glm::ivec3(0, 0, static_cast<int>(CHUNK_LENGTH)));
            return {left, front, right, back};
          };

          auto __chunks = get_neighbors(p);
          _chunk->Render(1, firstRun, __chunks[0].lock(), __chunks[1].lock(),
                         __chunks[2].lock(), __chunks[3].lock());
        } else {
          // Within current chunk
          _chunk->Render(1, firstRun, m_chunks[i + 1][j], m_chunks[i][j + 1],
                         m_chunks[i - 1][j], m_chunks[i][j - 1]);
        }
      }
      if (m_RenderIter.load() == BIOMESTATUS::SETUP) {
        m_chunksSetup.fetch_add(1, std::memory_order_release);
      } else if (m_RenderIter.load() == BIOMESTATUS::REFRESH) {
        m_chunksRerendered.fetch_add(1, std::memory_order_release);
      } else if (m_RenderIter.load() == BIOMESTATUS::FINAL) {
        m_chunksFinished.fetch_add(1, std::memory_order_release);
      } else {
        assert(false);
      }
    }
  }
}

Biome::Biome(int t, glm::ivec3 pos, GLboolean display) {
  m_type = t;
  m_biomePos = pos;
  m_displayBiome = display;
  uint64_t x = pos.x / BIOME_LENGTH;
  uint64_t y = pos.y / BIOME_HEIGHT;
  uint64_t z = pos.z / BIOME_LENGTH;
  m_id = static_cast<uint64_t>(BIOME_COUNTX * BIOME_COUNTZ) * y +
         static_cast<uint64_t>(BIOME_COUNTX) * x + z;

  m_dirtyBit = false;
  allocate_chunks(true);

  for (int i = 0; i < CHUNK_COUNTX; i++) {
    for (int j = 0; j < CHUNK_COUNTZ; j++) {
      m_dirtyBit |= m_chunks[i][j]->m_dirtyBit;
    }
  }
}

void Biome::RecycleBiome(int t, glm::ivec3 pos, GLboolean display) {
  m_type = t;
  m_biomePos = pos;
  m_displayBiome = display;
  uint64_t x = pos.x / BIOME_LENGTH;
  uint64_t y = pos.y / BIOME_HEIGHT;
  uint64_t z = pos.z / BIOME_LENGTH;
  m_id = static_cast<uint64_t>(BIOME_COUNTX * BIOME_COUNTZ) * y +
         static_cast<uint64_t>(BIOME_COUNTX) * x + z;

  m_dirtyBit = false;
  m_chunksSetup = 0;
  m_chunksFinished = 0;
  m_chunksRerendered = 0;
  m_RenderIter = BIOMESTATUS::IDLE;

  m_renderQueue.clear();
  allocate_chunks(false);

  for (int i = 0; i < CHUNK_COUNTX; i++) {
    for (int j = 0; j < CHUNK_COUNTZ; j++) {
      m_dirtyBit |= m_chunks[i][j]->m_dirtyBit;
    }
  }
}

Biome::~Biome() {
  // terminate worker threads
  m_running = false;

  if (m_worker1.joinable())
    m_worker1.join();

  if (m_worker2.joinable())
    m_worker2.join();
}

void Biome::SetupBiome(bool firstRun) {
  if (!m_displayBiome)
    return;

  if (firstRun) {
    if (m_worker1.joinable())
      m_worker1.join();
    m_RenderIter.store(BIOMESTATUS::SETUP);
    m_worker1 = std::thread([this, firstRun]() { setup_chunks(firstRun); });
  } else {
    if (m_worker2.joinable())
      m_worker2.join();
    m_worker2 = std::thread([this, firstRun]() { setup_chunks(firstRun); });
  }

  auto pos = m_biomePos + glm::ivec3(HALF_BLOCK_SIZE);
  auto biome = world->get_biome_by_center(pos);
  world->m_bindQueue.push(biome);
}

void Biome::Draw(OBJ_TYPE type, glm::dvec3 cameraPos) {
  for (auto [_, weak_chunk] : m_renderQueue) {
    if (auto chunk = weak_chunk.lock()) {
      chunk->Draw(type, cameraPos);
    } else {
      std::cerr << "[ERROR] BIOME::Draw chunk is null\n";
      continue;
    }
  }
}

std::array<Plane, 6> ExtractFrustumPlanes(const glm::dmat4 &m) {
  std::array<Plane, 6> planes;

  // Extract rows from column-major matrix
  glm::dvec4 row0 = glm::dvec4(m[0][0], m[1][0], m[2][0], m[3][0]);
  glm::dvec4 row1 = glm::dvec4(m[0][1], m[1][1], m[2][1], m[3][1]);
  glm::dvec4 row2 = glm::dvec4(m[0][2], m[1][2], m[2][2], m[3][2]);
  glm::dvec4 row3 = glm::dvec4(m[0][3], m[1][3], m[2][3], m[3][3]);

  // Left
  planes[0].normal = glm::dvec3(row3 + row0);
  planes[0].d = (row3 + row0).w;

  // Right
  planes[1].normal = glm::dvec3(row3 - row0);
  planes[1].d = (row3 - row0).w;

  // Bottom
  planes[2].normal = glm::dvec3(row3 + row1);
  planes[2].d = (row3 + row1).w;

  // Top
  planes[3].normal = glm::dvec3(row3 - row1);
  planes[3].d = (row3 - row1).w;

  // Near
  planes[4].normal = glm::dvec3(row3 + row2);
  planes[4].d = (row3 + row2).w;

  // Far
  planes[5].normal = glm::dvec3(row3 - row2);
  planes[5].d = (row3 - row2).w;

  for (auto &p : planes)
    p.normalize();

  return planes;
}

bool AABBInFrustum(const std::array<Plane, 6> &planes, const glm::dvec3 &min,
                   const glm::dvec3 &max) {
  for (const auto &plane : planes) {
    glm::dvec3 positive;

    positive.x = (plane.normal.x >= 0) ? max.x : min.x;
    positive.y = (plane.normal.y >= 0) ? max.y : min.y;
    positive.z = (plane.normal.z >= 0) ? max.z : min.z;

    if (plane.distance(positive) < 0)
      return false;
  }
  return true;
}

void Biome::Update_queue(glm::dvec3 playerpos, glm::dmat4 VP) {
  bool chunk_visible = false;
  auto planes = ExtractFrustumPlanes(VP);
  for (int i = 0; i < CHUNK_COUNTX; i++) {
    for (int j = 0; j < CHUNK_COUNTZ; j++) {
      auto chunk = m_chunks[i][j];
      if (!chunk)
        continue;
      glm::ivec3 cpos = chunk->m_chunkPos;

      // Chunk center
      glm::ivec3 center =
          cpos + glm::ivec3(CHUNK_LENGTH, CHUNK_HEIGHT, CHUNK_LENGTH) / 2;

      // Distance check
      float dx = abs(playerpos.x - center.x), dz = abs(playerpos.z - center.z);
      bool inRange = dx <= RENDER_DISTANCE + CHUNK_LENGTH &&
                     dz <= RENDER_DISTANCE + CHUNK_LENGTH;

      // If player is literally inside the chunk
      bool insideChunk =
          (playerpos.x >= cpos.x && playerpos.x < cpos.x + CHUNK_LENGTH) &&
          (playerpos.z >= cpos.z && playerpos.z < cpos.z + CHUNK_LENGTH);

      glm::ivec3 min = chunk->m_chunkPos;
      glm::ivec3 max =
          min + glm::ivec3(CHUNK_LENGTH, CHUNK_HEIGHT, CHUNK_LENGTH);

      // If any of the chunk in range
      if (inRange) {
        chunk_visible = true;
      }
      // Final decision
      bool inView = AABBInFrustum(planes, min, max);
      if (inView || insideChunk) {
        chunk->m_displayChunk = 1;
      } else {
        chunk->m_displayChunk = 0;
      }
    }
  }

  // If none of the chunks are visible for a biome remove it
  if (!chunk_visible) {
    glm::ivec3 bps = {
        m_biomePos.x / BIOME_LENGTH,
        m_biomePos.y / BIOME_HEIGHT,
        m_biomePos.z / BIOME_LENGTH,
    };

    // Only mark for removal after both the threads are done setting up
    constexpr auto count = CHUNK_COUNTX * CHUNK_COUNTZ;
    bool t_setupCompleted = m_chunksFinished == count;
    // check if can be removed
    if (world->m_biomes.isPresent(m_id)) {
      // Removing chunk
      m_Run = t_setupCompleted;
      world->m_biomes.set(bps.x, bps.y, bps.z, false);
    }
  }
}

void Biome::save(std::string _save_file) {
  std::string path = "save/tmp/" + _save_file + ".bin";
  std::ofstream save_file(path.c_str(), std::ios::binary | std::ios::trunc);
  // Save All the dirty chunks
  for (int k = 0; k < CHUNK_COUNTZ; k++) {
    for (int l = 0; l < CHUNK_COUNTX; l++) {
      auto chunk = m_chunks[k][l];
      if (!chunk || !chunk->m_dirtyBit)
        continue;
      world->m_saveMap.emplace(chunk->m_saveId, chunk);
    }
  }

  int count = world->m_saveMap.size();
  save_file.write(reinterpret_cast<char *>(&count), sizeof(count));
  for (auto [id, chunk_weak] : world->m_saveMap) {
    if (auto chunk = chunk_weak.lock()) {
      chunk->Serialize(save_file);
    }
  }
  std::cout << "Biome Saved\n";
}
