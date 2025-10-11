#include "Biome.h"

#include <fstream>

#include "Renderer.h"
#include "World.h"

extern std::unique_ptr<World> world;

static void
load_p(decltype(Biome::chunks) &chunks, glm::ivec3 &Biomepos, int i, bool display, int type) {
  for (int j = 0; j < CHUNK_COUNTZ; j++) {
    int idx = CHUNK_COUNTZ * i + j;
    chunks[i][j] = std::make_shared<Chunk>(idx, Biomepos, glm::ivec3(i, 0, j), display, type);
  }
}

static void render_p(decltype(Biome::chunks) &chunks, int i, bool firstRun) {
  for (int j = 0; j < CHUNK_COUNTZ; j++) {
    int idx = CHUNK_COUNTZ * i + j;
    auto _chunk = chunks[i][j];
    if (firstRun) {
      _chunk->Render(1, firstRun, nullptr, nullptr, nullptr, nullptr);
    } else {
      if (i == 0 || i == CHUNK_COUNTX - 1 || j == 0 || j == CHUNK_COUNTZ - 1) {
        // Have to check neigbouring biome
        // get the center of chunks 1st block
        glm::ivec3 p = _chunk->chunkpos + glm::ivec3(HALF_BLOCK_SIZE);
        auto get_neighbors = [](glm::ivec3 vec) -> std::vector<std::shared_ptr<Chunk>> {
          std::shared_ptr<Chunk> left, front, right, back;
          left = world->get_chunk_by_center(
              vec + glm::ivec3(static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE), 0, 0));
          front = world->get_chunk_by_center(
              vec + glm::ivec3(0, 0, static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE)));
          right = world->get_chunk_by_center(
              vec - glm::ivec3(static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE), 0, 0));
          back = world->get_chunk_by_center(
              vec - glm::ivec3(0, 0, static_cast<int>(CHUNK_BLOCK_COUNT * BLOCK_SIZE)));
          return {left, front, right, back};
        };

        auto __chunks = get_neighbors(p);
        _chunk->Render(1, firstRun, __chunks[0], __chunks[1], __chunks[2], __chunks[3]);
      } else {
        // Within current chunk
        _chunk->Render(
            1, firstRun, chunks[i + 1][j], chunks[i][j + 1], chunks[i - 1][j], chunks[i][j - 1]);
      }
    }
    auto biome = world->get_biome_by_center(_chunk->chunkpos + glm::ivec3(HALF_BLOCK_SIZE));
    biome->chunks_ready.fetch_add(1, std::memory_order_relaxed);
  }
}

Biome::Biome(int t, glm::ivec3 pos, GLboolean display) {
  type = t;
  Biomepos = pos;
  displaybiome = display;

  GLuint idx = 0;
  dirtybit = false;
  std::array<std::thread, CHUNK_COUNTX> threads;
  for (int i = 0; i < CHUNK_COUNTX; i++) {
    threads[i] = std::thread(load_p, std::ref(chunks), std::ref(Biomepos), i, true, t);
  }

  // Joining is slow
  for (auto &thread : threads) thread.join();

  for (int i = 0; i < CHUNK_COUNTX; i++) {
    for (int j = 0; j < CHUNK_COUNTZ; j++) {
      dirtybit |= chunks[i][j]->dirtybit;
    }
  }
}

void Biome::RenderBiome(bool firstRun) {
  if (!displaybiome) return;

  for (int i = 0; i < CHUNK_COUNTX; i++) {
    std::thread thread = std::thread(render_p, std::ref(chunks), i, firstRun);
    thread.detach();
  }

  auto biome = world->get_biome_by_center(Biomepos + glm::ivec3(HALF_BLOCK_SIZE));
  {
    std::lock_guard<std::mutex> lock(world->biome_mutex);
    world->bind_queue.push(biome);
  }
}

void Biome::Draw() {
  for (auto chunk : render_queue) {
    if (!chunk) {
      std::cerr << "[ERROR] BIOME::Draw chunk is null\n";
      continue;
    }
    chunk->Draw();
  }
}

void Biome::Update_queue(glm::vec3 playerpos, glm::vec3 playerForward, float fov) {
  float cosHalfFOV = cos(glm::radians(fov) / 2.0f);
  // Flatten forward to XZ plane
  glm::vec3 forwardXZ = glm::normalize(glm::vec3(playerForward.x, 0.0f, playerForward.z));
  for (int i = 0; i < CHUNK_COUNTX; i++) {
    for (int j = 0; j < CHUNK_COUNTZ; j++) {
      auto chunk = chunks[i][j];
      if (!chunk) continue;
      glm::vec3 cpos = chunk->chunkpos;

      float CHUNK_SIZE = CHUNK_BLOCK_COUNT * BLOCK_SIZE;
      // Chunk center
      glm::vec3 center = cpos + glm::vec3(CHUNK_SIZE / 2.0f, 0.0f, CHUNK_SIZE / 2.0f);

      glm::vec3 corners[4] = {
          cpos,                                        // bottom-left
          cpos + glm::vec3(CHUNK_SIZE, 0, 0),          // bottom-right
          cpos + glm::vec3(0, 0, CHUNK_SIZE),          // top-left
          cpos + glm::vec3(CHUNK_SIZE, 0, CHUNK_SIZE)  // top-right
      };

      bool inView = false;
      for (int k = 0; k < 4; k++) {
        glm::vec3 toCorner = glm::normalize(corners[k] - glm::vec3(playerpos.x, 0.0f, playerpos.z));
        if (glm::dot(forwardXZ, toCorner) > cosHalfFOV) {
          inView = true;
          break;  // no need to check other corners
        }
      }

      // Distance check
      float distXZ = glm::length(glm::vec2(playerpos.x - center.x, playerpos.z - center.z));
      bool inRange = distXZ <= RENDER_DISTANCE;

      // If player is literally inside the chunk
      bool insideChunk = (playerpos.x >= cpos.x && playerpos.x < cpos.x + CHUNK_SIZE) &&
                         (playerpos.z >= cpos.z && playerpos.z < cpos.z + CHUNK_SIZE);

      // Final decision
      if ((inRange && inView) || insideChunk) {
        chunk->displaychunk = 1;
      } else {
        chunk->displaychunk = 0;
      }
    }
  }
}
