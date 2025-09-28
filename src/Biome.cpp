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
    auto &_chunk = chunks[i][j];
    if (firstRun)
      _chunk->Render(1, firstRun, nullptr, nullptr, nullptr, nullptr);
    else {
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
  for (auto &thread : threads) thread.join();
  for (int i = 0; i < CHUNK_COUNTX; i++) {
    for (int j = 0; j < CHUNK_COUNTZ; j++) {
      dirtybit |= chunks[i][j]->dirtybit;
    }
  }
}

void Biome::RenderBiome(bool firstRun) {
  if (!displaybiome) return;

  GLuint idx = 0;
  std::array<std::thread, CHUNK_COUNTX> threads;
  for (int i = 0; i < CHUNK_COUNTX; i++) {
    threads[i] = std::thread(render_p, std::ref(chunks), i, firstRun);
  }

  for (auto &thread : threads) thread.join();

  // All openGL calls are to be made from main thread
  for (int i = 0; i < CHUNK_COUNTX; i++) {
    for (int j = 0; j < CHUNK_COUNTZ; j++) {
      auto &chunk = chunks[i][j];
      chunk->chunkva = std::make_unique<VertexArray>();
      chunk->chunkva->Bind();
      VertexBufferLayout layout;
      layout.Push(GL_UNSIGNED_INT, 1);
      VertexBuffer vb(chunk->cube_vertices.data(), chunk->cube_vertices.size() * sizeof(GLuint));
      chunk->chunkva->AddBuffer(vb, layout);
      IndexBuffer ib(chunk->cube_indices.data(), chunk->cube_indices.size());
      //  glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
      render_queue.insert(chunk);
    }
  }
}

void Biome::Draw() {
  for (auto &chunk : render_queue) {
    chunk->Draw();
  }
}

void Biome::Update_queue(glm::vec3 playerpos) {
  for (int i = 0; i < CHUNK_COUNTX; i++) {
    for (int j = 0; j < CHUNK_COUNTZ; j++) {
      auto &chunk = chunks[i][j];
      glm::vec3 cpos = chunk->chunkpos;

      // If not displaying currently
      if (render_queue.find(chunk) == render_queue.end()) {
        int X = abs(static_cast<int>(playerpos.x - cpos.x)),
            Z = abs(static_cast<int>(playerpos.z - cpos.z));
        if (X <= RENDER_DISTANCE || (Z <= RENDER_DISTANCE)) {
          chunk->displaychunk = 1;
        } else {
          chunk->displaychunk = 0;  // Do not display farther than RENDER_DISTANCE
        }
      } else {  // If diplaying currently
        int X = abs(static_cast<int>(playerpos.x - cpos.x)),
            Z = abs(static_cast<int>(playerpos.z - cpos.z));
        if ((X > RENDER_DISTANCE) || (Z > RENDER_DISTANCE)) {
          chunk->displaychunk = 0;
        } else {
          chunk->displaychunk = 1;  // Keep displaying near than RENDER_DISTANCE
        }
      }
    }
  }
}
