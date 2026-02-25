#include "Biome.h"

#include <fstream>

#include "Renderer.h"
#include "World.h"

extern std::unique_ptr<World> world;

static void load_p(decltype(Biome::chunks) &chunks, glm::ivec3 &Biomepos,
                   bool display, int type) {
  for (int i = 0; i < CHUNK_COUNTX; i++) {
    for (int j = 0; j < CHUNK_COUNTZ; j++) {
      int idx = CHUNK_COUNTX * i + j;
      chunks[i][j] = std::make_shared<Chunk>(idx, Biomepos, glm::ivec3(i, 0, j),
                                             display, type);
    }
  }
}

static void render_p(decltype(Biome::chunks) &chunks, bool firstRun) {
  for (int i = 0; i < CHUNK_COUNTX; i++) {
    for (int j = 0; j < CHUNK_COUNTZ; j++) {
      auto _chunk = chunks[i][j];
      if (firstRun) {
        _chunk->Render(1, firstRun, nullptr, nullptr, nullptr, nullptr);
      } else {
        if (i == 0 || i == CHUNK_COUNTX - 1 || j == 0 ||
            j == CHUNK_COUNTZ - 1) {
          // Have to check neigbouring biome
          // get the center of chunks 1st block
          glm::ivec3 p = _chunk->chunkpos + glm::ivec3(HALF_BLOCK_SIZE);
          auto get_neighbors =
              [](glm::ivec3 vec) -> std::vector<std::shared_ptr<Chunk>> {
            std::shared_ptr<Chunk> left, front, right, back;
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
          _chunk->Render(1, firstRun, __chunks[0], __chunks[1], __chunks[2],
                         __chunks[3]);
        } else {
          // Within current chunk
          _chunk->Render(1, firstRun, chunks[i + 1][j], chunks[i][j + 1],
                         chunks[i - 1][j], chunks[i][j - 1]);
        }
      }
      auto biome = world->get_biome_by_center(_chunk->chunkpos +
                                              glm::ivec3(HALF_BLOCK_SIZE));
      if (biome)
        biome->chunks_ready.fetch_add(1, std::memory_order_release);
    }
  }
}

Biome::Biome(int t, glm::ivec3 pos, GLboolean display) {
  type = t;
  Biomepos = pos;
  displaybiome = display;

  dirtybit = false;
  load_p(chunks, Biomepos, true, t);

  for (int i = 0; i < CHUNK_COUNTX; i++) {
    for (int j = 0; j < CHUNK_COUNTZ; j++) {
      dirtybit |= chunks[i][j]->dirtybit;
    }
  }
}

void Biome::SetupBiome(bool firstRun) {
  if (!displaybiome)
    return;

  if (firstRun) {
    worker1 = std::thread(render_p, std::ref(chunks), firstRun);
  } else {
    worker2 = std::thread(render_p, std::ref(chunks), firstRun);
  }

  auto biome =
      world->get_biome_by_center(Biomepos + glm::ivec3(HALF_BLOCK_SIZE));
  {
    std::lock_guard<std::mutex> lock(world->biome_mutex);
    world->bind_queue.push(biome);
  }
}

void Biome::Draw(OBJ_TYPE type) {
  for (auto chunk : render_queue) {
    if (!chunk) {
      std::cerr << "[ERROR] BIOME::Draw chunk is null\n";
      continue;
    }
    chunk->Draw(type);
  }
}

std::array<Plane, 6> ExtractFrustumPlanes(const glm::mat4 &m) {
  std::array<Plane, 6> planes;

  // Left
  planes[0].normal.x = m[0][3] + m[0][0];
  planes[0].normal.y = m[1][3] + m[1][0];
  planes[0].normal.z = m[2][3] + m[2][0];
  planes[0].d = m[3][3] + m[3][0];

  // Right
  planes[1].normal.x = m[0][3] - m[0][0];
  planes[1].normal.y = m[1][3] - m[1][0];
  planes[1].normal.z = m[2][3] - m[2][0];
  planes[1].d = m[3][3] - m[3][0];

  // Bottom
  planes[2].normal.x = m[0][3] + m[0][1];
  planes[2].normal.y = m[1][3] + m[1][1];
  planes[2].normal.z = m[2][3] + m[2][1];
  planes[2].d = m[3][3] + m[3][1];

  // Top
  planes[3].normal.x = m[0][3] - m[0][1];
  planes[3].normal.y = m[1][3] - m[1][1];
  planes[3].normal.z = m[2][3] - m[2][1];
  planes[3].d = m[3][3] - m[3][1];

  // Near
  planes[4].normal.x = m[0][3] + m[0][2];
  planes[4].normal.y = m[1][3] + m[1][2];
  planes[4].normal.z = m[2][3] + m[2][2];
  planes[4].d = m[3][3] + m[3][2];

  // Far
  planes[5].normal.x = m[0][3] - m[0][2];
  planes[5].normal.y = m[1][3] - m[1][2];
  planes[5].normal.z = m[2][3] - m[2][2];
  planes[5].d = m[3][3] - m[3][2];

  for (auto &p : planes)
    p.normalize();

  return planes;
}

bool AABBInFrustum(const std::array<Plane, 6> &planes, const glm::vec3 &min,
                   const glm::vec3 &max) {
  for (const auto &plane : planes) {
    glm::vec3 positive;

    positive.x = (plane.normal.x >= 0) ? max.x : min.x;
    positive.y = (plane.normal.y >= 0) ? max.y : min.y;
    positive.z = (plane.normal.z >= 0) ? max.z : min.z;

    if (plane.distance(positive) < 0)
      return false;
  }
  return true;
}

void Biome::Update_queue(glm::vec3 playerpos, glm::mat4 VP) {
  for (int i = 0; i < CHUNK_COUNTX; i++) {
    for (int j = 0; j < CHUNK_COUNTZ; j++) {
      auto chunk = chunks[i][j];
      if (!chunk)
        continue;
      glm::vec3 cpos = chunk->chunkpos;

      float CHUNK_SIZE = CHUNK_BLOCK_COUNT * BLOCK_SIZE;
      // Chunk center
      glm::vec3 center =
          cpos + glm::vec3(CHUNK_SIZE / 2.0f, 0.0f, CHUNK_SIZE / 2.0f);

      // Distance check
      float distXZ = glm::length(
          glm::vec2(playerpos.x - center.x, playerpos.z - center.z));
      bool inRange = distXZ <= RENDER_DISTANCE;

      // If player is literally inside the chunk
      bool insideChunk =
          (playerpos.x >= cpos.x && playerpos.x < cpos.x + CHUNK_SIZE) &&
          (playerpos.z >= cpos.z && playerpos.z < cpos.z + CHUNK_SIZE);

      glm::vec3 min = chunk->chunkpos;
      glm::vec3 max = min + glm::vec3(CHUNK_SIZE);

      auto planes = ExtractFrustumPlanes(VP);
      // Final decision
      if ((inRange && AABBInFrustum(planes, min, max)) || insideChunk) {
        chunk->displaychunk = 1;
      } else {
        chunk->displaychunk = 0;
      }
    }
  }
}
